import pyvista as pv
import numpy as np
from scipy.spatial.distance import cdist
from sklearn.decomposition import PCA
import networkx as nx

def compute_edge_probability(mesh, edge_start, edge_end, curvature_weight=0.1, angle_weight=0.1, length_scale=1.0):
    """
    Compute the probability of an edge based on length, curvature, and angular deviation.

    Parameters:
        mesh (pv.PolyData): The suzrface mesh.
        edge_start (ndarray): Start point of the edge.
        edge_end (ndarray): End point of the edge.
        curvature_weight (float): Weight for curvature contribution.
        angle_weight (float): Weight for angle contribution.
        length_scale (float): Scaling factor for edge length.

    Returns:
        float: Probability of the edge.
    """
    edge_vector = edge_end - edge_start
    edge_length = np.linalg.norm(edge_vector)

    # Compute curvature (approximate as deviation from mean surface normal)
    start_normal = mesh.point_normals[mesh.find_closest_point(edge_start)]
    end_normal = mesh.point_normals[mesh.find_closest_point(edge_end)]
    curvature = 1 - np.dot(start_normal, end_normal)

    # Compute angular deviation (cosine similarity)
    angle_deviation = np.abs(np.dot(edge_vector / edge_length, start_normal))

    # Edge probability
    probability = np.exp(-edge_length ** 2 / (2 * length_scale ** 2)) * (1 + curvature_weight * curvature + angle_weight * angle_deviation)
    print(probability)
    return probability

def trace_boundary(mesh, boundary_points):
    """
    Perform boundary tracing using the APBT algorithm.

    Parameters:
        mesh (pv.PolyData): The surface mesh.
        boundary_points (list): List of points defining the boundary.

    Returns:
        boundary_path (list): List of traced points forming the boundary.
        boundary_faces (set): Set of face IDs along the boundary.
    """
    surface = mesh.extract_surface()
    graph = nx.Graph()

    # Build graph representation of the mesh
    edges = surface.extract_all_edges()
    for i in range(edges.n_cells):
        edge_points = edges.get_cell(i).points
        edge_start, edge_end = edge_points[0], edge_points[1]
        probability = compute_edge_probability(surface, edge_start, edge_end)

        # Convert to positive weight: lower weight = higher probability
        weight = 1 / max(probability, 1e-6)  # Avoid division by 0
        graph.add_edge(
            surface.find_closest_point(edge_start),
            surface.find_closest_point(edge_end),
            weight=weight
        )

    # Trace boundary using shortest path in the graph
    closest_point_ids = [surface.find_closest_point(pt) for pt in boundary_points]
    boundary_path = []
    boundary_faces = set()

    for i in range(len(closest_point_ids)):
        start = closest_point_ids[i]
        end = closest_point_ids[(i + 1) % len(closest_point_ids)]  # Wrap to form closed loop

        # Compute shortest path
        try:
            path = nx.shortest_path(graph, source=start, target=end, weight="weight")
        except nx.NetworkXNoPath:
            print(f"Warning: No path found between point {i} and point {(i + 1) % len(closest_point_ids)}")
            continue

        boundary_path.extend(surface.points[path])

        # Collect boundary faces
        for j in range(len(path) - 1):
            start_id, end_id = path[j], path[j + 1]

            for cell_id in range(surface.n_cells):
                cell = surface.get_cell(cell_id)
                cell_point_ids = set(cell.point_ids)

                if {start_id, end_id}.issubset(cell_point_ids):
                    boundary_faces.add(cell_id)



    return np.array(boundary_path), boundary_faces


def multi_resolution_tracing(mesh, boundary_points, levels=3):
    """
    Perform multi-resolution tracing.

    Parameters:
        mesh (pv.PolyData): The surface mesh.
        boundary_points (list): List of points defining the boundary.
        levels (int): Number of resolution levels.

    Returns:
        final_path (list): Traced boundary points at the finest resolution.
    """
    hierarchy = [mesh]
    for _ in range(1, levels):
        hierarchy.append(hierarchy[-1].decimate_pro(0.5))

    coarse_path, _ = trace_boundary(hierarchy[0], boundary_points)
    for level in range(1, levels):
        coarse_path = trace_boundary(hierarchy[level], coarse_path)[0]

    return coarse_path

# def visualize_boundary(mesh, boundary_points, boundary_faces):
#     """
#     Visualize the mesh, traced boundary, and associated faces.

#     Parameters:
#         mesh (pv.PolyData): The surface mesh.
#         boundary_points (ndarray): Traced boundary points.
#         boundary_faces (set): Set of boundary face IDs.
#     """
#     face_ids = np.array(list(boundary_faces), dtype=int)

#     if len(face_ids) == 0:
#         print("Warning: No boundary faces were found.")
#         return

#     boundary_poly = mesh.extract_cells(face_ids)

#     plotter = pv.Plotter()
#     plotter.add_mesh(mesh, color="cyan", opacity=0.5, show_edges=True, label="Original Mesh")
#     plotter.add_mesh(boundary_poly, color="green", label="Boundary Faces", show_edges=True)
#     plotter.add_points(boundary_points, color="yellow", point_size=10, label="Boundary Points")
#     plotter.add_legend()
#     plotter.show()

# Example usage
# if __name__ == "__main__":
#     file_path = "angle_depth_test/Momotaro.vtk"  # Replace with your mesh file

#     try:
#         mesh = pv.read(file_path)
#     except Exception as e:
#         raise RuntimeError(f"Failed to read the mesh file: {e}")

#     # Clean the mesh if necessary
#     mesh = mesh.clean()

#     # Interactive point selection for boundary points
#     boundary_points = []
#     plotter = pv.Plotter()
    
#     def on_pick_callback(point, picker=None):
#         if len(point) == 3:
#             boundary_points.append(point)
#             print(f"Picked Point: {point}")
#         if len(boundary_points) >= 6:
#             print("Tracing boundary...")
#             traced_boundary, boundary_faces = trace_boundary(mesh, boundary_points)
#             visualize_boundary(mesh, traced_boundary, boundary_faces)

#     plotter.enable_point_picking(callback=on_pick_callback, use_picker=True)
#     plotter.add_mesh(mesh, color="cyan", opacity=0.5, show_edges=True)
#     plotter.show()


#     if not boundary_points:
#         raise ValueError("No boundary points were selected. Please pick points on the mesh.")

#     # Trace the boundary and visualize
#     traced_boundary, boundary_faces = trace_boundary(mesh, boundary_points)
#     visualize_boundary(mesh, traced_boundary, boundary_faces)

