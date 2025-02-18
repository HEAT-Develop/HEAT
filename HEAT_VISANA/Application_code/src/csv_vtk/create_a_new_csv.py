import pyvista as pv
import pandas as pd
import numpy as np
import csv 
def combine_vtk_csv(csv_filename, output_filename, model_name):
    
    df = pd.read_csv(csv_filename)

    # print("First few rows of CSV data:")
    # print(df.head())

    existing_polygon_ids = set(df['polygon_id']) 
    # print("Existing Polygon IDs:")
    #print(existing_polygon_ids)

    new_rows = []

    if model_name == "200k":
        for i in range(1,196609): 
            if i not in existing_polygon_ids:

                new_row = {
                    'polygon_id': i,
                    'longitude': int(-1),   
                    'latitude': int(-1),    
                    'phase_angle': int(-1),  
                    'incidence_angle': int(-1),   
                    'emission_angle': int(-1),   
                    'pixel_x': int(-1),   
                    'pixel_y': int(-1),   
                    'brightness_temperature': 0.000000,   
                    'local_solar_time': int(-1),   
                    'local_solar_time_normal_vector': int(-1)   
                }
                new_rows.append(new_row)
    elif model_name =="49k":
        for i in range(1,49152): 
            if i not in existing_polygon_ids:

                new_row = {
                    'polygon_id': i,
                    'longitude': int(-1),   
                    'latitude': int(-1),    
                    'phase_angle': int(-1),  
                    'incidence_angle': int(-1),   
                    'emission_angle': int(-1),   
                    'pixel_x': int(-1),   
                    'pixel_y': int(-1),   
                    'brightness_temperature': 0.000000,   
                    'local_solar_time': int(-1),   
                    'local_solar_time_normal_vector': int(-1)   
                }
                new_rows.append(new_row)
    elif model_name =="800k":
        for i in range(1,786432): 
            if i not in existing_polygon_ids:

                new_row = {
                    'polygon_id': i,
                    'longitude': int(-1),   
                    'latitude': int(-1),    
                    'phase_angle': int(-1),  
                    'incidence_angle': int(-1),   
                    'emission_angle': int(-1),   
                    'pixel_x': int(-1),   
                    'pixel_y': int(-1),   
                    'brightness_temperature': 0.000000,   
                    'local_solar_time': int(-1),   
                    'local_solar_time_normal_vector': int(-1)  
                }
                new_rows.append(new_row)
        
    new_rows_df = pd.DataFrame(new_rows)
    
    updated_df = pd.concat([df, new_rows_df], ignore_index=True).sort_values(by='polygon_id')

    updated_df.to_csv(output_filename, index=False)
    print(f"Updated CSV saved to {output_filename}")
