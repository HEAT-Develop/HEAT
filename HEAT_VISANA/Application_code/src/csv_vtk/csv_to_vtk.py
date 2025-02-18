import pyvista as pv
import pandas as pd
import numpy as np

def extract_and_write_vtk(input_filename, output_filename, cell_data):

    mesh = pv.read(input_filename)
    
    points = mesh.points
    faces = mesh.faces.reshape(-1, 4)[:, 1:] 

    n_points = len(points)
    n_polygons = len(faces)
    n_total_polygon_indices = sum(len(polygon) for polygon in faces) + n_polygons 

    with open(output_filename, 'w') as file:
        file.write("# vtk DataFile Version 3.0\n")
        if n_polygons >=300000:
            file.write("SHAPE_SPC_800k_v20200323\n")
        elif n_polygons >=150000 <= 230000: 
            file.write("SHAPE_SPC_200k_v20200323\n")
        else:
            file.write("SHAPE_SPC_49k_v20200323\n")

        file.write("ASCII\n")
        file.write("DATASET POLYDATA\n")

        file.write(f"POINTS {n_points} float\n")
        for point in points:
            file.write(f"{' '.join(map(str, point))}\n")

        file.write(f"POLYGONS {n_polygons} {n_total_polygon_indices}\n")
        for polygon in faces:
            file.write(f"{len(polygon)} {' '.join(map(str, polygon))}\n")

        for field_name in cell_data.keys():
            if field_name == "brightness_temperature":
                num_cells = len(cell_data[field_name])
                file.write(f"CELL_DATA {n_polygons}\n")
                file.write(f"SCALARS {field_name} float\n")
                file.write("LOOKUP_TABLE default\n")
                for value in cell_data[field_name]:
                    file.write(f"{value}\n")
            else:
                file.write(f"POINT_DATA {n_points}\n")
                file.write(f"CELL_DATA {n_polygons}\n")
                file.write(f"SCALARS {field_name} float\n")
                file.write("LOOKUP_TABLE default\n")
                for value in cell_data[field_name]:
                    file.write(f"{value}\n")


    print(f"VTK file '{output_filename}' written successfully.")

def combine_vtk_csv(vtk_filename, csv_filename, output_filename):
    mesh = pv.read(vtk_filename)
    df = pd.read_csv(csv_filename)

    num_cells = mesh.n_cells
    num_points = mesh.n_points

    brightness_temperature = df["brightness_temperature"].values
    longitude = df["longitude"].values
    latitude = df["latitude"].values
    phase_angle = df["phase_angle"].values
    incidence_angle = df["incidence_angle"].values
    emission_angle = df["emission_angle"].values
    pixel_x = df["pixel_x"].values
    pixel_y = df["pixel_y"].values
    local_solar_time = df["local_solar_time"].values
    local_solar_time_normal_vector = df["local_solar_time_normal_vector"].values

    cell_data = {
        'brightness_temperature': brightness_temperature,
        'longitude': longitude,
        'latitude': latitude,
        'phase_angle': phase_angle,
        'incidence_angle': incidence_angle,
        'emission_angle': emission_angle,
        'pixel_x': pixel_x,
        'pixel_y': pixel_y,
        'local_solar_time': local_solar_time,
        'local_solar_time_normal_vector': local_solar_time_normal_vector
    }
    extract_and_write_vtk(vtk_filename, output_filename, cell_data)
