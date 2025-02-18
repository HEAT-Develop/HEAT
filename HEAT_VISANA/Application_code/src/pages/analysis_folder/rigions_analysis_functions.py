import numpy as np
import pyvista as pv

from functools import partial
from itertools import combinations

from sklearn.decomposition import PCA
from scipy.optimize import minimize
from scipy.interpolate import griddata

 

class MeshLoader:

    def __init__(self, file_path):

        self.file_path = file_path
        self._mesh = None

    def load_and_align_mesh(self):

        self._mesh = pv.read(self.file_path)
        self._mesh = self._mesh.cell_data_to_point_data()

        if self._mesh.n_points == 0:
            raise ValueError("The mesh does not contain any points.")

        vertices = self._mesh.points
        pca = PCA(n_components=3)
        pca.fit(vertices)
        principal_axes = pca.components_

        # rotation_matrix = np.eye(3)
        # if abs(principal_axes[0, 2]) > 0.5:
        #     rotation_matrix = np.array([principal_axes[1],
        #                                 principal_axes[0],
        #                                 principal_axes[2]])
        if principal_axes[2, 2] < 0:
            rotation_matrix = np.array([principal_axes[1], principal_axes[0], -principal_axes[2]])
        else:
            rotation_matrix = np.array([principal_axes[1], principal_axes[0], principal_axes[2]])

        rotated_vertices = np.dot(vertices, rotation_matrix.T)
        self._mesh.points = rotated_vertices

        xy_centroid = np.mean(self._mesh.points[:, :2], axis=0)
        self._mesh.translate(-np.array([*xy_centroid, 0]))

        z_coords = self._mesh.points[:, 2]
        self._mesh.point_data['Z-Gradient'] = z_coords

        self._print_extreme_points()

    @property
    def mesh(self):
  
        return self._mesh

    def _print_extreme_points(self):
  
        z_vals = self._mesh.points[:, 2]
        highest_pt = self._mesh.points[np.argmax(z_vals)]
        lowest_pt = self._mesh.points[np.argmin(z_vals)]



class MeshAnalyzer:
  
    def __init__(self, mesh):

        self.mesh = mesh

    # -------------------------- Geometric/Analysis Methods -------------------------- #
    def compute_geodesic_paths(self, points):
       
        surface = self.mesh.extract_surface()
        point_ids = [surface.find_closest_point(pt) for pt in points]
        all_combinations = combinations(point_ids, 2)

        path_vertices = {}
        for pair in all_combinations:
            try:
                geodesic_segment = surface.geodesic(pair[0], pair[1])
                path_vertices[pair] = geodesic_segment.points
            except Exception as e:
                print(f"Error computing geodesic path for {pair}: {e}")
        return path_vertices

    @staticmethod
    def find_path_intersection(path1, path2):
        
        set1 = set(map(tuple, path1))
        set2 = set(map(tuple, path2))
        return list(set1 & set2)

    def trace_boundary(self, boundary_points):
       
        surface = self.mesh.extract_surface()
        closest_ids = [surface.find_closest_point(pt) for pt in boundary_points]

        traced_points = []
        unique_points = set()
        boundary_face_ids = set()
        path_vertices = {}

        for i in range(len(closest_ids)):
            curr_id = closest_ids[i]
            next_id = closest_ids[(i + 1) % len(closest_ids)]
            key = tuple(sorted((curr_id, next_id)))

            try:
                geo_path = surface.geodesic(curr_id, next_id)
                path_points = geo_path.points
                path_vertices[key] = path_points

                for j in range(len(path_points) - 1):
                    start_pt = path_points[j]
                    end_pt = path_points[j + 1]
                    id_s = surface.find_closest_point(start_pt)
                    id_e = surface.find_closest_point(end_pt)
                    edge_ids = {id_s, id_e}

                    for f_id in range(surface.n_cells):
                        cell = surface.get_cell(f_id)
                        if edge_ids.issubset(cell.point_ids):
                            boundary_face_ids.add(f_id)
                            break

                traced_points.extend(path_points)
                unique_points.update(map(tuple, path_points))

            except Exception as e:
                print(f"Error computing geodesic path between {curr_id} and {next_id}: {e}")

        traced_points = np.array(traced_points)
        traced_points = np.vstack([traced_points, traced_points[0]])

        poly_edges = pv.PolyData()
        poly_edges.points = traced_points
        cells = []
        for i in range(len(traced_points) - 1):
            cells.append([2, i, i + 1])

        cells.append([2, len(traced_points)-1, 0])
        poly_edges.lines = np.hstack(cells)

        unique_boundary_points = np.array(list(unique_points))
        return poly_edges, unique_boundary_points, boundary_face_ids, path_vertices

    @staticmethod
    def fit_circle_to_points(points):
       
        xy = points[:, :2]

        def circle_cost(params):
            cx, cy, r = params
            dists = np.linalg.norm(xy - np.array([cx, cy]), axis=1)
            return np.sum((dists - r)**2)

        center_init = np.mean(xy, axis=0)
        radius_init = np.mean(np.linalg.norm(xy - center_init, axis=1))
        initial_guess = (*center_init, radius_init)

        result = minimize(circle_cost, initial_guess)
        cx, cy, r = result.x
        return (cx, cy), r

    @staticmethod
    def generate_circular_boundary(center, radius, num_points, original_points):
      
        angles = np.linspace(0, 2*np.pi, num_points, endpoint=False)
        x = center[0] + radius*np.cos(angles)
        y = center[1] + radius*np.sin(angles)

        orig_xy = original_points[:, :2]
        orig_z = original_points[:, 2]
        circle_xy = np.column_stack((x, y))

        z = griddata(orig_xy, orig_z, circle_xy, method='nearest')
        return np.column_stack((x, y, z))

    def find_closest_points_on_mesh(self, points):
      
        mesh_pts = self.mesh.points
        closest_pts = []
        for p in points:
            dists = np.linalg.norm(mesh_pts - p, axis=1)
            idx = np.argmin(dists)
            closest_pts.append(mesh_pts[idx])
        return np.array(closest_pts)

    @staticmethod
    def compute_centroid_of_points(points):
       
        return np.mean(points, axis=0)

    def find_vertical_intersection(self, start_point):
      
        direction = np.array([0, 0, -1])
        end_point = start_point + direction * 1000
        pts, _ = self.mesh.ray_trace(start_point, end_point)
        if len(pts) == 0:
            raise ValueError("No intersection found downward from the start_point.")
        return pts[0]

    def iterate_to_boundary(self, start_point, boundary_points):
       
        surface = self.mesh.extract_surface()
        visited = set()
        inside_faces = set()
        boundary_faces = set()

        start_face = surface.find_closest_cell(start_point)
        queue = [start_face]

        while queue:
            curr_face_id = queue.pop(0)
            if curr_face_id in visited:
                continue
            visited.add(curr_face_id)

            face_pts = surface.extract_cells([curr_face_id]).points
            is_boundary = any(
                np.any(np.linalg.norm(face_pts - bp, axis=1) < 1e-3)
                for bp in boundary_points
            )
            if is_boundary:
                boundary_faces.add(curr_face_id)
            else:
                inside_faces.add(curr_face_id)
                neighbors = surface.cell_neighbors(curr_face_id)
                queue.extend([n for n in neighbors if n not in visited])

        inside_faces.update(boundary_faces)
        return inside_faces

    def compute_centroid_of_cells(self, selected_cells):
        
        surface = self.mesh.extract_surface()
        cell_points = []
        for c_id in selected_cells:
            cell = surface.get_cell(c_id)
            cell_points.extend(cell.points)
        cell_points = np.array(cell_points)
        return np.mean(cell_points, axis=0)

    def compute_crater_slope(self, inside_faces, method='average'):
        
        surface = self.mesh.extract_surface()
        region_polydata = surface.extract_cells(list(inside_faces))
        
        if not isinstance(region_polydata, pv.PolyData):
            region_polydata = region_polydata.extract_geometry()
        
        region_polydata.compute_normals(cell_normals=True, point_normals=False, inplace=True)
        normals = region_polydata.cell_data['Normals']
        vertical = np.array([0, 0, 1])
        
        norms = np.linalg.norm(normals, axis=1)
        norms[norms < 1e-12] = 1.0  # Avoid division by zero
        normals_normalized = normals / norms[:, None]
        
        dots = np.dot(normals_normalized, vertical)
        dots = np.clip(dots, -1.0, 1.0)
        
        slopes_deg = np.degrees(np.arccos(dots))
        
        return np.mean(slopes_deg) if method == 'average' else np.max(slopes_deg)

    def compute_geodesic_diameter(self,boundary_points):
        # Get the surface from your mesh
        surface = self.mesh.extract_surface()
        point_ids = [surface.find_closest_point(pt) for pt in boundary_points]
        max_distance = 0.0
        geodesic_paths = []  
        longest_geodesic = None
        
        for i in range(len(point_ids)):
            for j in range(i+1, len(point_ids)):
                try:
                    geodesic_path = surface.geodesic(point_ids[i], point_ids[j])
                    path_length = geodesic_path.length 
                    geodesic_paths.append((geodesic_path, path_length))

                    if path_length > max_distance:
                        max_distance = path_length
                        longest_geodesic = geodesic_path

                      
                except Exception as e:
                    print(f"Error computing geodesic between points {i} and {j}: {e}")

        
        return max_distance

    


class MeshVisualizer:
   
    MAX_PICKS = 6

    def __init__(self, mesh_loader, analyzer):
      
        self.mesh_loader = mesh_loader
        self.analyzer = analyzer
        self.picked_points = [] 

    def start_interactive_plot(self):
       
        mesh = self.mesh_loader.mesh
        if mesh is None:
            raise ValueError("Mesh is not loaded. Check MeshLoader first.")

        z_vals = mesh.points[:, 2]
        highest = mesh.points[np.argmax(z_vals)]
        lowest = mesh.points[np.argmin(z_vals)]
        pick_callback = partial(self._on_point_picked)
      
    def _on_point_picked(self, point, index):

        self.picked_points.append(point)

        if len(self.picked_points) == self.MAX_PICKS:
            self._run_analysis_and_visualization()

    def _run_analysis_and_visualization(self):

        mesh = self.mesh_loader.mesh
        analyzer = self.analyzer

        path_vertices = analyzer.compute_geodesic_paths(self.picked_points)

        point_ids = [mesh.find_closest_point(pt) for pt in self.picked_points]
        num_p = len(point_ids)
        pair_ac = (point_ids[0], point_ids[num_p // 2])
        pair_bd = (point_ids[num_p // 4], point_ids[(3 * num_p) // 4])

        if pair_ac in path_vertices and pair_bd in path_vertices:
            intersection_ac = analyzer.find_path_intersection(
                path_vertices[pair_ac],
                path_vertices[pair_bd]
            )

            _, boundary_points, boundary_face_ids, _ = analyzer.trace_boundary(
                np.array(self.picked_points)
            )

            (cx, cy), radius = analyzer.fit_circle_to_points(boundary_points)



            circle_pts = analyzer.generate_circular_boundary(
                (cx, cy),
                radius,
                num_points=len(boundary_points),
                original_points=boundary_points
            )
            closest_pts = analyzer.find_closest_points_on_mesh(circle_pts)

            poly_edges2, boundary_pts_2, boundary_fids2, _ = analyzer.trace_boundary(
                closest_pts
            )

            if intersection_ac:

                if intersection_ac:
                    start_seed = np.array(intersection_ac[0])
                else:
                    start_seed = boundary_pts_2[0]

                inside_faces = analyzer.iterate_to_boundary(
                    start_seed, boundary_pts_2
                )

                geodesic_diameter = analyzer.compute_geodesic_diameter(boundary_pts_2)
                
                self.plot_depth_angle_between_A_B(inside_faces)
        else:
            print(f"Missing keys: {pair_ac} or {pair_bd} not found in path_vertices.")

    def run_analysis_without_plotting(self, picked_points):

        analyzer = self.analyzer
        mesh = self.mesh_loader.mesh
        results = {}

        path_vertices = analyzer.compute_geodesic_paths(picked_points)
        results["Path Keys"] = list(path_vertices.keys())

        point_ids = [mesh.find_closest_point(pt) for pt in picked_points]
        num_p = len(point_ids)
        pair_ac = (point_ids[0], point_ids[num_p // 2])
        pair_bd = (point_ids[num_p // 4], point_ids[(3 * num_p) // 4])
        if pair_ac in path_vertices and pair_bd in path_vertices:
            intersection_ac = analyzer.find_path_intersection(
                path_vertices[pair_ac],
                path_vertices[pair_bd]
            )
        else:
            intersection_ac = None
        results["Intersection (AC vs BD)"] = intersection_ac

        _, boundary_points, _, _ = analyzer.trace_boundary(np.array(picked_points))
        results["Boundary Points"] = boundary_points

        (cx, cy), radius = analyzer.fit_circle_to_points(boundary_points)
        results["Circle Center (XY)"] = (cx, cy)
        results["Circle Radius"] = radius
        crater_diameter = 2 * radius * 1000  # converting to meters, if needed
        results["Crater Diameter from the Circle"] = crater_diameter

        circle_pts = analyzer.generate_circular_boundary((cx, cy), radius, num_points=len(boundary_points), original_points=boundary_points)
        closest_pts = analyzer.find_closest_points_on_mesh(circle_pts)
        _, boundary_pts_2, _, _ = analyzer.trace_boundary(closest_pts)
        if intersection_ac and len(intersection_ac) > 0:
            start_seed = np.array(intersection_ac[0])
        else:
            start_seed = boundary_pts_2[0]
        inside_faces = analyzer.iterate_to_boundary(start_seed, boundary_pts_2)
        results["Inside Faces"] = inside_faces

        geodesic_diameter = analyzer.compute_geodesic_diameter(boundary_pts_2)
        results["Geodesic Diameter"] = geodesic_diameter * 1000  # convert to meters if needed
        results["Geodesic Radius"] = (geodesic_diameter / 2) * 1000

        angle_deg, depth_m, average_slope = self.compute_depth_angle(inside_faces)
        results["Angle with Horizontal"] = angle_deg
        results["Depth"] = depth_m
        results["Average Crater Slope"] = average_slope

        centroid = analyzer.compute_centroid_of_cells(inside_faces)
        results["Centroid (C)"] = centroid

        return results

    def compute_depth_angle(self, inside_faces):

        mesh = self.mesh_loader.mesh
        surface = mesh.extract_surface()

        if not inside_faces:
            raise ValueError("No faces found in the selected region.")

        selected_polydata = surface.extract_cells(list(inside_faces))
        region_points = selected_polydata.points

        C = self.analyzer.compute_centroid_of_cells(inside_faces)
        try:
            intersection_point = self.analyzer.find_vertical_intersection(C)
        except ValueError as e:
            raise ValueError("Vertical intersection failed: " + str(e))
        A_index = mesh.find_closest_point(intersection_point)
        A = mesh.points[A_index]
        B = region_points[np.argmax(region_points[:, 2])]

        depth = B[2] - A[2]
        delta_xyz = B - A
        delta_xy = np.linalg.norm(delta_xyz[:2])
        angle_deg = np.degrees(np.arctan2(delta_xyz[2], delta_xy)) if delta_xy > 1e-12 else 0.0
        average_slope = self.analyzer.compute_crater_slope(inside_faces, method='average')

        return angle_deg, depth, average_slope


    #-------Method to measure angle & depth from the region's centroid-------------#
    def plot_depth_angle_between_A_B(self, inside_faces):
    
        mesh = self.mesh_loader.mesh
        surface = mesh.extract_surface()

        if not inside_faces:
            print("No faces found in the selected region.")
            return

        selected_polydata = surface.extract_cells(list(inside_faces))
        region_points = selected_polydata.points

        C = self.analyzer.compute_centroid_of_cells(inside_faces)

        try:
            intersection_point = self.analyzer.find_vertical_intersection(C)
        except ValueError as e:
            print("Vertical intersection failed:", e)
            return

        A_index = mesh.find_closest_point(intersection_point)
        A = mesh.points[A_index]

        B = region_points[np.argmax(region_points[:, 2])]

        depth = B[2] - A[2]

        delta_xyz = B - A
        delta_z = delta_xyz[2]
        delta_xy = np.linalg.norm(delta_xyz[:2])
        angle_rad = 0.0
        angle_deg = 0.0

        if delta_xy > 1e-12:
            angle_rad = np.arctan2(delta_z, delta_xy)
            angle_deg = np.degrees(angle_rad)

        average_slope = self.analyzer.compute_crater_slope(inside_faces, method='average')

        region_polydata = surface.extract_cells(list(inside_faces))
        if not isinstance(region_polydata, pv.PolyData):
            region_polydata = region_polydata.extract_geometry()

        region_polydata.compute_normals(cell_normals=True, point_normals=False, inplace=True)
        normals = region_polydata.cell_data['Normals']
        vertical = np.array([0, 0, 1])
        crater_slopes = []
        for n in normals:
            norm_val = np.linalg.norm(n)
            if norm_val > 1e-12:
                n_normalized = n / norm_val
            else:
                n_normalized = n
            dot = np.dot(n_normalized, vertical)
            dot = np.clip(dot, -1.0, 1.0) 
            angle_rad_cell = np.arccos(dot)
            slope_deg = np.degrees(angle_rad_cell)
            crater_slopes.append(slope_deg)
        crater_slopes = np.array(crater_slopes)
        region_polydata.cell_data["Crater_Slope"] = crater_slopes
