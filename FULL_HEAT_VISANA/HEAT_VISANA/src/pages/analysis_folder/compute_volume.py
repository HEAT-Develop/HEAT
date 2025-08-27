import pyvista as pv
import numpy as np


def compute_volume_of_region(mesh: pv.PolyData, face_ids: set,true_rim_points, reference_plane='auto') -> float:

    surface = mesh.extract_surface()
    region = surface.extract_cells(list(face_ids))

    # Determine reference height
    if reference_plane == 'auto':
        rim_points = true_rim_points
        
        ref_z = np.mean(rim_points[:, 2])
    else:
        ref_z = float(reference_plane)

    # Compute signed volume of prisms beneath each triangle
    total_volume = 0.0
    for cell_id in face_ids:
        cell = surface.get_cell(cell_id)
        pts = cell.points
        if len(pts) != 3:
            continue  # skip non-triangular faces
        v0, v1, v2 = pts

        # Average height below the reference plane
        h0 = ref_z - v0[2]
        h1 = ref_z - v1[2]
        h2 = ref_z - v2[2]

        # Triangle area using cross product
        area = 0.5 * np.linalg.norm(np.cross(v1 - v0, v2 - v0))

        # Prism volume under this triangle
        prism_vol = area * (h0 + h1 + h2) / 3.0
        total_volume += prism_vol

    return abs(total_volume)

def debug_volume_check(mesh: pv.PolyData, inside_faces: set,t_rim_points, ref_z: float = None):
    """
    Visual debug visualization for crater volume estimation.

    Args:
        mesh (pv.PolyData): The full mesh.
        inside_faces (set): Set of face IDs that define the crater interior.
        ref_z (float, optional): Reference plane height. If None, computed from rim points.
    """
    surface = mesh.extract_surface()

    # Extract inside faces
    crater_region = surface.extract_cells(list(inside_faces))

    # Extract rim points from boundary of the region
    rim_points = t_rim_points
    # for cell_id in inside_faces:
    #     cell = surface.get_cell(cell_id)
    #     pts = cell.points
    #     rim_points.append(pts)
    rim_points = np.vstack(rim_points)
    rim_points = np.unique(rim_points, axis=0)

    # Calculate reference height if needed
    if ref_z is None:
        ref_z = np.mean(rim_points[:, 2])
    
    # Create a reference plane for visualization
    bounds = crater_region.bounds
    plane = pv.Plane(
        center=( (bounds[0] + bounds[1]) / 2,
                 (bounds[2] + bounds[3]) / 2,
                 ref_z),
        direction=(0,0,1),
        i_size=(bounds[1] - bounds[0]) * 1.2,
        j_size=(bounds[3] - bounds[2]) * 1.2,
    )

    # Plot everything
    plotter = pv.Plotter()
    plotter.add_mesh(surface, color="white", opacity=0.3, show_edges=False, label="Full Surface")
    plotter.add_mesh(crater_region, color="green", opacity=0.8, show_edges=True, label="Inside Faces")
    plotter.add_points(rim_points, color="red", point_size=10, render_points_as_spheres=True, label="Rim Points")
    plotter.add_mesh(plane, color="blue", opacity=0.5, label="Reference Plane")

    plotter.add_legend()
    plotter.show()