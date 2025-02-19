from PyQt6.QtWidgets import QLabel
from PyQt6.QtCore import Qt

from PyQt6.QtWidgets import QLabel, QApplication,QMessageBox, QPushButton, QVBoxLayout, QWidget, QHBoxLayout,QDateTimeEdit, QComboBox, QSizePolicy
from PyQt6.QtCore import Qt,QDateTime
import sys
from pyvistaqt import BackgroundPlotter,QtInteractor
import matplotlib.pyplot as plt
from datetime import datetime
import os
import pyvista as pv 
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.widgets import RectangleSelector
from matplotlib.backends.backend_qtagg import NavigationToolbar2QT as NavigationToolbar
import numpy as np
import cartopy.crs as ccrs
import cartopy.feature as cfeature
import matplotlib.colors as mcolors
import csv
import json
import pandas as pd
np.set_printoptions(threshold=np.inf) 
from qtconsole.rich_jupyter_widget import RichJupyterWidget
from qtconsole.manager import QtKernelManager

module_path = 'HEAT_VISANA'
if module_path not in sys.path:
    sys.path.append(module_path)
db_path = "FULL_HEAT_VISANA/HEAT_VISANA/db/db.py"

if not os.path.exists(db_path):
    raise FileNotFoundError(f"Error: 'db.py' not found at {db_path}")
from db.db import DataBase
db_instance = DataBase()



class RigionType():
    def __init__(self):
        self.rigionId = int # Selection ID
        self.rigion_name = str # Selection Name
        self.pickle_file = str  # Pickel file?
        self.model_type = str # Model Type
        self.model_size = str # Model Size
        self.rigion_date = str # Date of file
        self.unstructuredg_rid = pv.UnstructuredGrid # CELL ID
        # Points(x,y,z)
        self.save_as_vtk = None # file path of selection saved
        self.cell_ids = []
        self.brightness_temperature = []
        self.emission_angle = []
        self.incidence_angle = []
        self.phase_angle = []
        self.local_solar = []
        self.local_solar_time_normal_vector = []
        self.csv_path = str # CSV file
        self.pixel_x = []
        self.pixel_y = []
        self.longitude = []
        self.latitude = []
        self.points = []
        self.polygons = []
        
    def add_db():
        print("saved data in the DB")

    def create_csv(self, data):
        print("csv created correctly")
        output_csv_file = "data_for_analysis.csv"

        headers = [
            "region_id", "latitude", "longitude", "timestamp", 
            "brightness_temperature", "phase_angle", "incidence_angle", 
            "emission_angle", "local_solar_time", "local_solar_time_normal_vector",
            "x","y","z","nx","ny","nz","poly_id"
        ]
        with open(output_csv_file, mode='w', newline='') as csv_file:
            writer = csv.DictWriter(csv_file, fieldnames=headers)
            
            # Write the header row
            writer.writeheader()
            
            # Write each data entry
            for entry in data:
                x_value = entry.get("x", 0)
                if isinstance(x_value, pv.pyvista_ndarray):
                    # If x is an array, check its length
                    x_value = x_value.item() if x_value.size == 1 else x_value[0]  # Taking the first element if array
                else:
                    x_value = float(x_value)

                y_value = entry.get("y", 0)
                if isinstance(y_value, pv.pyvista_ndarray):
                    y_value = y_value.item() if y_value.size == 1 else y_value[0]
                else:
                    y_value = float(y_value)

                z_value = entry.get("z", 0)
                if isinstance(z_value, pv.pyvista_ndarray):
                    z_value = z_value.item() if z_value.size == 1 else z_value[0]
                else:
                    z_value = float(z_value)

                # Handling nx, ny, nz (numpy.float32)
                nx_value = entry.get("nx", 0)
                if isinstance(nx_value, (np.float32, np.float64)):
                    nx_value = float(nx_value)
                else:
                    nx_value = float(nx_value)

                ny_value = entry.get("ny", 0)
                if isinstance(ny_value, (np.float32, np.float64)):
                    ny_value = float(ny_value)
                else:
                    ny_value = float(ny_value)

                nz_value = entry.get("nz", 0)
                if isinstance(nz_value, (np.float32, np.float64)):
                    nz_value = float(nz_value)
                else:
                    nz_value = float(nz_value)
                poly_id = entry.get("poly_id",0)
                if isinstance(poly_id, (np.float32, np.float64)):
                    poly_id = int(poly_id)
                else:
                    poly_id = int(poly_id)

                # view_factors = entry.get("view_factors", [])
                # print(view_factors)
                # # Convert non-serializable types (e.g., tuples, numpy types) into native Python types
                # for factor in view_factors:
                #     if isinstance(factor.get("cell_pair"), tuple):
                #         factor["cell_pair"] = list(factor["cell_pair"])  # Convert tuple to list
                    
                #     # Convert numpy data types (e.g., np.int64, np.float32) to native Python types
                #     factor["distance"] = float(factor["distance"])  # Convert np.float32 to float
                #     factor["angle"] = float(factor["angle"])  # Convert np.float32 to float
                #     factor["cell_pair"] = [int(x) for x in factor["cell_pair"]]  # Convert np.int64 to int

                
                # if isinstance(view_factors, (np.float32, np.float64)):
                #     view_factors = float(view_factors)
                # else:
                #     view_factors = float(view_factors)

                writer.writerow({
                    "region_id": entry.get("region_id", "N/A"),
                    "latitude": float(entry.get("latitude", 0)),
                    "longitude": float(entry.get("longitude", 0)),
                    "timestamp": entry.get("timestamp", "N/A"),  # Default to "N/A" if missing
                    "brightness_temperature": float(entry.get("brightness_temperature", 0)),
                    "phase_angle": float(entry.get("phase_angle", 0)),
                    "incidence_angle": float(entry.get("incidence_angle", 0)),
                    "emission_angle": float(entry.get("emission_angle", 0)),
                    "local_solar_time": float(entry.get("local_solar_time", 0)),
                    "local_solar_time_normal_vector": float(entry.get("local_solar_time_normal_vector", 0)),
                    "x": x_value,
                    "y": y_value,
                    "z": z_value,
                    "nx": nx_value,
                    "ny": ny_value,
                    "nz": nz_value,
                    # "view_factors": view_factors,
                    "poly_id": poly_id
                })

        print(f"Data has been successfully written to {output_csv_file}")

    def initialize_data_structure():
        data = []
        return data

    def all_similar_rigions_data(self):
        all_files = db_instance.get_all_names(self.model_type,self.model_size)
        print(len(all_files))
        dic = {}
        base_data = []
        region = 0 
        print(len(all_files))
        for i in range (2):
                mesh = pv.read(all_files[i]) 
                print(f"Processing file {all_files[i]}, {i}...")
        
                required_arrays = ['latitude', 'longitude', 'brightness_temperature', 
                                'phase_angle', 'incidence_angle', 'emission_angle',
                                'local_solar_time', 'local_solar_time_normal_vector']
                
                if not all(arr in mesh.array_names for arr in required_arrays):
                    #print(f"One or more required arrays missing in file {all_files[i]}. Skipping...")
                    continue
                if 'Normals' not in mesh.cell_data:
                    #print("Normals NOT found in the mesh. Computing...")
                    mesh_with_normals = mesh.compute_normals(cell_normals=True)
                    cell_normals = mesh_with_normals.cell_data['Normals']
                else:
                    cell_normals = mesh.cell_data['Normals']
                
               

                for cell_id in self.cell_ids:  
                    if 0 <= cell_id < mesh['latitude'].size: 
                        lat_ids = mesh['latitude'][cell_id] 
                        lon_ids = mesh['longitude'][cell_id]
                        temp_ids = mesh['brightness_temperature'][cell_id]
                        phase_ids = mesh["phase_angle"][cell_id]
                        incidence_ids = mesh["incidence_angle"][cell_id]
                        emission_ids = mesh["emission_angle"][cell_id]
                        solar_ids = mesh["local_solar_time"][cell_id]
                        solar_normal_ids = mesh["local_solar_time_normal_vector"][cell_id]

                        file_name = all_files[i].split("/")[-1]
                        timestamp_raw = file_name.split("_")[2] + file_name.split("_")[3]
                        dt = datetime.strptime(timestamp_raw, "%Y%m%d%H%M%S")
                        timestamp_iso = dt.isoformat() + "Z"
                        cell = mesh.get_cell(cell_id)
                        point_ids = cell.point_ids 
                        points = mesh.points

                        x, y, z = points[point_ids[0]], points[point_ids[1]], points[point_ids[2]] 


                        nx, ny, nz = cell_normals[cell_id]

                        if lat_ids != -1 and lon_ids != -1: 
                            found = False
                            current_dic = {}

                            for data in base_data:
                                if (
                                    lat_ids in np.atleast_1d(data["latitude"]) and
                                    lon_ids in np.atleast_1d(data["longitude"])
                                ):

                                    current_dic["region_id"] = data["region_id"]
                                    current_dic["latitude"] = lat_ids
                                    current_dic["longitude"] = lon_ids
                                    current_dic["timestamp"] = timestamp_iso
                                    current_dic["brightness_temperature"] = temp_ids
                                    current_dic["phase_angle"] = phase_ids
                                    current_dic["incidence_angle"] = incidence_ids
                                    current_dic["emission_angle"] = emission_ids
                                    current_dic["local_solar_time"] = solar_ids
                                    current_dic["local_solar_time_normal_vector"] = solar_normal_ids
                                    current_dic["x"] = x
                                    current_dic["y"] = y
                                    current_dic["z"] = z
                                    current_dic["nx"] = nx
                                    current_dic["ny"] = ny
                                    current_dic["nz"] = nz
                                    current_dic["poly_id"]  = cell_id

                                    found = True
                                    break

                            if not found:
                                current_dic["region_id"] = region
                                current_dic["latitude"] = lat_ids
                                current_dic["longitude"] = lon_ids
                                current_dic["timestamp"] = timestamp_iso
                                current_dic["brightness_temperature"] = temp_ids
                                current_dic["phase_angle"] = phase_ids
                                current_dic["incidence_angle"] = incidence_ids
                                current_dic["emission_angle"] = emission_ids
                                current_dic["local_solar_time"] = solar_ids
                                current_dic["local_solar_time_normal_vector"] = solar_normal_ids
                                current_dic["x"] = x
                                current_dic["y"] = y
                                current_dic["z"] = z
                                current_dic["nx"] = nx
                                current_dic["ny"] = ny
                                current_dic["nz"] = nz
                                current_dic["poly_id"]  = cell_id

                                region += 1

                            base_data.append(current_dic)
                    else:
                        print(f"cell_id {cell_id} is out of bounds for mesh with size {mesh['latitude'].size}. Skipping...")

        self.create_csv(base_data)

     
        print("created correctly")
    

    def create_vtk(self):
        
        print("This is here: ",self.unstructuredg_rid)

    def write_vtk_polydata(self, filename):
       
        original_points = self.points 
        

        points = self.points
        
        polygons = self.polygons
        temperatures = self.brightness_temperature
        emission_angle = self.emission_angle
        incidence_angle = self.incidence_angle
        phase_angle = self.phase_angle
        local_solar_time = self.local_solar
        local_solar_time_normal_vector = self.local_solar_time_normal_vector
        pixel_x = self.pixel_x
        pixel_y = self.pixel_y
        longitude = self.longitude
        latitude = self.latitude
        n_points = len(points)
        n_polygons = len(polygons)
        n_total_polygon_indices = sum(len(polygon) + 1 for polygon in polygons)  # +1 for the size prefix
   
   
        with open(filename, 'w') as file:
            file.write("# vtk DataFile Version 3.0\n")
            file.write(f"SHAPE_{self.model_type}_{self.model_size}_v20200323\n")  # Example header
            file.write("ASCII\n")
            file.write("DATASET POLYDATA\n")
            file.write(f"POINTS {n_points} float\n")
            
            for point in points:
                file.write(f"{' '.join(map(str, point))}\n")
            
            file.write(f"POLYGONS {n_polygons} {n_total_polygon_indices}\n")
            for polygon in polygons:
                file.write(f"{len(polygon)} {' '.join(map(str, polygon))}\n")
            
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS brightness_temperature float\n")
            file.write("LOOKUP_TABLE default\n")
            for temp in temperatures:
                file.write(f"{temp}\n")
            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS emission_angle float\n")
            file.write("LOOKUP_TABLE default\n")
            for e_a in emission_angle:
                file.write(f"{e_a}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS incidence_angle float\n")
            file.write("LOOKUP_TABLE default\n")
            for i_a in incidence_angle:
                file.write(f"{i_a}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS phase_angle float\n")
            file.write("LOOKUP_TABLE default\n")
            for p_a in phase_angle:
                file.write(f"{p_a}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS local_solar_time float\n")
            file.write("LOOKUP_TABLE default\n")
            for l_s in local_solar_time:
                file.write(f"{l_s}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS local_solar_time_normal_vector float\n")
            file.write("LOOKUP_TABLE default\n")
            for l_s_n_v in local_solar_time_normal_vector:
                file.write(f"{l_s_n_v}\n")
            
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS pixel_x float\n")
            file.write("LOOKUP_TABLE default\n")
            for p_x in pixel_x:
                file.write(f"{p_x}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS pixel_y float\n")
            file.write("LOOKUP_TABLE default\n")
            for p_y in pixel_y:
                file.write(f"{p_y}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS longitude float\n")
            file.write("LOOKUP_TABLE default\n")
            for lon in longitude:
                file.write(f"{lon}\n")

            file.write(f"POINT_DATA {n_points}\n")
            file.write(f"CELL_DATA {n_polygons}\n")
            file.write("SCALARS latitude float\n")
            file.write("LOOKUP_TABLE default\n")
            for lat in latitude:
                file.write(f"{lat}\n")
            
class PyVistaWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.layout = QVBoxLayout(self)
        self.plotter = QtInteractor(self)
        self.layout.addWidget(self.plotter.interactor)
        self.setLayout(self.layout)
        
        
    def update_plot(self, mesh,d):
        self.plotter.clear()
        curvature = mesh.curvature(curv_type='mean')
        mesh.point_data['Curvature'] = curvature
        self.plotter.add_mesh(mesh,show_edges=False,cmap='coolwarm',scalars='Curvature')
        self.plotter.update()

def create_ipython_widget():

    ipython_widget = RichJupyterWidget()

    kernel_manager = QtKernelManager()

    kernel_manager.kernel_cmd = [
        sys.executable,   
        "-m", "ipykernel_launcher",
        "-f", "{connection_file}"
    ]

    kernel_manager.start_kernel()
    kernel_client = kernel_manager.client()
    kernel_client.start_channels()

    ipython_widget.kernel_manager = kernel_manager
    ipython_widget.kernel_client = kernel_client

    ipython_widget.banner = (
        "Embedded IPython Console\n"
        "Running under the same interpreter as your main PyQt app.\n"
        "Pre-importing libraries...\n"
    )

    libraries_to_import = """
    %gui qt5
    import sys
    import os
    import numpy as np
    np.set_printoptions(threshold=np.inf) 
    import pandas as pd
    from numpy import array
    import pyvista as pv
    import cartopy.crs as ccrs
    import cartopy.feature as cfeature
    import matplotlib
    import matplotlib.pyplot as plt
    import matplotlib.colors as mcolors
    import csv
    import json
    from numpy import array, linspace,float32
    """

    kernel_client.execute(libraries_to_import)

    return ipython_widget

class Selections(QWidget):
    def __init__(self, parent=None):
        super().__init__( parent)

        self.console = create_ipython_widget()
        self.initUI()
        self.setGeometry(100, 100, 1260, 900)
        self.rigion_type = RigionType()
        self.colorbar = None 
        self.selected_cells =[]
        self.points_stack = []
        self.polygons_stack =[]
        self.temperatures_stack = []
        self.latitude_stack = []
        self.longitude_stack = []
        self.incidence_angle_stack = [] 
        self.phase_angle_stack = [] 
        self.local_solar_stack = []  
        self.pixel_x_stack = [] 
        self.pixel_y_stack = [] 
        self.longitude_stack = []  
        self.latitude_stack = []  
        self.local_solar_time_normal_vector_stack = [] 
        self.selected_cells_stack = []
        self.emission_angle_stack =[]

        self.data_x = None
        self.data_y = None
        self.data_vals = None
        self.rectangle_selector = None
        
        self.test_console_execution()
        self.mesh = None
        self.console_expanded = False 


    def initUI(self):
        self.load_css('FULL_HEAT_VISANA/HEAT_VISANA/src/style/multiview.css') 

        main_layout = QHBoxLayout(self)
        left_layout = QVBoxLayout()
        right_layout = QVBoxLayout()

       

        #------RIGHT-LYOUT------#

        self.mesh = None

        header_layout = QHBoxLayout()
        self.date_time_edit = QDateTimeEdit(QDateTime.currentDateTime())
        self.model_type_combo = QComboBox()
        self.model_size_combo = QComboBox()
        self.btn_send_date = QPushButton("Start")
        self.scalar_type_combo = QComboBox()

                #-----DATA----#

        self.date_time_edit.setDisplayFormat("yyyy-MM-dd HH:mm:ss")
        self.date_time_edit.setCalendarPopup(True)
        self.allowed_dates, dates_range = self.set_valid_dates()
        if dates_range:
            min_date = dates_range[0]
            max_date = dates_range[-1]
            self.date_time_edit.setMinimumDate(min_date)
            self.date_time_edit.setMaximumDate(max_date)
        else:
            print("No valid date range found, skipping date range setup.")



        self.date_time_edit.dateTimeChanged.connect(lambda: self.check_date())
                
                #-----END----#
                
                #----MODEL----#

        self.model_type_combo.addItems(["sfm", "spc"])
        self.model_size_combo.addItems(["50k", "200k", "800k"])
        self.model_type_combo.currentIndexChanged.connect(self.update_model_size)

                #----END----#


                #-SEND-DATE-#
        self.btn_send_date.clicked.connect(self.send_date)
                #----END----#

                #-SCALAR-DATA-#
        self.scalar_type_combo.currentIndexChanged.connect(self.updateVisualizationType)
                #----END----#

        header_layout.addWidget(self.date_time_edit)
        header_layout.addWidget(self.model_type_combo)
        header_layout.addWidget(self.model_size_combo)
        header_layout.addWidget(self.btn_send_date)
        header_layout.addWidget(self.scalar_type_combo)
                #---END---#

        sub_header_layout = QHBoxLayout()
        self.btn_new_selection = QPushButton()
        self.btn_select_ID = QComboBox()
        self.btn_reset = QPushButton("Clean selection")

        sub_header_layout.addWidget(self.btn_new_selection)
        #sub_header_layout.addWidget(self.btn_select_ID)
        sub_header_layout.addWidget(self.btn_reset)

            #-NEW-SELECTION-#
        self.btn_new_selection.setText("Start a New Selection")
        self.btn_new_selection.clicked.connect(self.new_selection)
        self.selecting = False

        self.btn_reset.clicked.connect(self.clean_contiune_select)
        self.continue_selecting = False
            #-----END-----#

        model_layout = QHBoxLayout()
        self.view = PyVistaWidget()
        model_layout.addWidget(self.view)

        bottom_layout = QHBoxLayout()
        btn_update_db = QPushButton()
        btn_download_csv = QPushButton("Download CSV")
        btn_download_vtk = QPushButton("Download vtk")

        #bottom_layout.addWidget(btn_update_db)
        bottom_layout.addWidget(btn_download_csv)
        bottom_layout.addWidget(btn_download_vtk)

        btn_download_vtk.clicked.connect(self.download_vtk)
        btn_download_csv.clicked.connect(self.download_csv)

        left_layout.addLayout(header_layout)
        left_layout.addLayout(sub_header_layout)
        left_layout.addLayout(model_layout)
        left_layout.addLayout(bottom_layout)

        #----------END----------#

        #------RIGH-LAYOUT------#

        top_layout = QHBoxLayout()
        btn_plot = QPushButton("ADD PLOT")
        self.combo_scalar_data = QComboBox()
        self.combo_visualization = QComboBox()

        self.console_toggle_button = QPushButton("Show Console")  # Button to toggle console visibility
        


        top_layout.addWidget(btn_plot)
        top_layout.addWidget(self.combo_scalar_data)
        top_layout.addWidget(self.combo_visualization)
        top_layout.addWidget(self.console_toggle_button)

        plot_layout = QHBoxLayout()
        
        self.fig, self.ax = plt.subplots( layout="constrained")
        self.canvas = FigureCanvas(self.fig)
        self.ax.clear()
        self.console = create_ipython_widget()
        self.console.setVisible(False)

        right_layout.addLayout(top_layout)
        right_layout.addLayout(plot_layout)
        plot_layout.addWidget(self.canvas)
        right_layout.addWidget(self.console)

        btn_plot.clicked.connect(self.plot)
        self.console_toggle_button.clicked.connect(self.toggle_console)

        self.combo_scalar_data.addItems(['brightness_temperature',  'phase_angle', 'incidence_angle', 'emission_angle', 'local_solar_time', 'local_solar_time_normal_vector'])
        self.combo_visualization.addItems(['longitude/latitude', 'pixel_x/pixel_y'])
        self.combo_visualization.currentIndexChanged.connect(self.update)

        # Avoid Zeros or -1 depening the data (Implement funcion, this data does not exist,
        #  the user can select if want to avoid non data or want to use this data)
        # Last function to develop, first ask in HEAT DV team if it is needed
        # Another solution the user can do zoom in the plot (ez but I personally dont like idk why)
        #  I think is easier not exaiting.
        #---------END--------#

        left_widget = QWidget()
        left_widget.setLayout(left_layout)

        right_widget = QWidget()
        right_widget.setLayout(right_layout)

        main_layout.addWidget(left_widget, 3) 
        main_layout.addWidget(right_widget, 2)

        self.setLayout(main_layout)


    # -----------IPython-Console-Helpers------------
    def toggle_console(self):
        if self.console_expanded:
            self.console.setVisible(False) 
            self.console_toggle_button.setText("Show Console")  
        else:
            self.console.setVisible(True) 
            self.console_toggle_button.setText("Hide Console")

        self.console_expanded = not self.console_expanded

    def refresh_plot(self):
        
        self.plot()
    def plot_console_data(self, x, y, values, label="Console Data"):
     
        self.ax.clear()

        scatter = self.ax.scatter(x, y, c=values, cmap='turbo', s=10)

        if self.colorbar:
            try:
                self.colorbar.remove()
            except:
                pass
            self.colorbar = None

        self.colorbar = self.fig.colorbar(scatter, ax=self.ax, shrink=0.6, label=label)

        self.ax.set_title(label)
        self.ax.set_xlabel("X data")
        self.ax.set_ylabel("Y data")

        self.canvas.draw()
    def push_to_console(self, var_dict):
        if not self.console:
            return
        
        kernel_client = self.console.kernel_client

        for var_name, var_value in var_dict.items():
            assignment_code = f"{var_name} = {repr(var_value)}"
            kernel_client.execute(assignment_code)

    def push_data(self, **kwargs):
        #print("DEBUG: push_data called with:", kwargs.keys())
        self.push_to_console(kwargs)

        code_snippet = f'''
    print("Just pushed variables: {list(kwargs.keys())}")
    '''
        self.console.kernel_client.execute(code_snippet)
        
        self.console.kernel_client.execute("simple_var = 42")
        self.console.kernel_client.execute('print("simple_var set to", simple_var)')


    def test_console_execution(self):
  
        if not self.console:
            #print("No console available!")
            return

        test_snippet = "print('Hello from test_console_execution!'); test_var = 42"
        
        kernel_client = self.console.kernel_client
        
        #print("DEBUG: Sending test snippet to embedded console...")

        msg_id = kernel_client.execute(test_snippet)  # Non-blocking
        #print(f"DEBUG: snippet sent with msg_id={msg_id}")


    #------------DOWNLAD CSV---------------------#


    def download_csv(self):
        print("CELLIDS ",self.rigion_type.cell_ids)
        self.rigion_type.all_similar_rigions_data()


    #------------END-------------------#

        #---------NEW-SELECTION--------#
    
    def new_selection(self):
        if not self.selecting:
            self.btn_new_selection.setText("Stop Selection")
            self.start_selection()
            self.selecting = True

        else:
            self.btn_new_selection.setText("Start a New Selection")
            self.stop_selection()
            self.selecting = False

    def stop_selection(self):
        try:
            # Disable picking
            self.view.plotter.disable_picking()
            print("Cell picking disabled.")

            self.view.plotter.enable()
            self.view.plotter.update()  

            self.selected_cells = [] 
        except Exception as e:
            print(f"Error while stopping selection: {e}")

    def start_selection(self):
        if self.mesh is None:
            print("No valid mesh loaded. Load a mesh before starting selection.")
            return

        if isinstance(self.mesh, pv.MultiBlock):
            self.mesh = self.mesh[0] if self.mesh.n_blocks > 0 else None

        if not self.mesh or self.mesh.n_cells == 0:
            print("The mesh is invalid or has no cells.")
            return
        self.view.plotter.clear()
        curvature = self.mesh.curvature(curv_type='mean')
        clamped_curvature = np.clip(curvature, -100, 200)
        self.mesh.point_data['Curvature'] = clamped_curvature

        self.view.plotter.add_mesh(self.mesh,  show_edges=False,cmap='coolwarm',scalars='Curvature')

        

        try:
            self.view.plotter.enable_cell_picking(
                callback=self.callback_cell_pick,
                through=False,
                show_message=False,
                color="blue",
                start=True
         
            )
            print("Cell picking enabled.")
        except Exception as e:
            print(f"Error enabling selection: {e}")

    def clean_contiune_select(self):
        if self.continue_selecting:

            self.selected_cells = []
            self.selected_cells =[]
            self.points_stack = []
            self.polygons_stack =[]
            self.temperatures_stack = []
            self.latitude_stack = []
            self.longitude_stack = []
            self.incidence_angle_stack = [] 
            self.phase_angle_stack = []  
            self.local_solar_stack = []  
            self.pixel_x_stack = [] 
            self.pixel_y_stack = []
            self.longitude_stack = [] 
            self.latitude_stack = []  
            self.local_solar_time_normal_vector_stack = [] 
            self.emission_angle_stack = []
            self.continue_selecting = False
            self.btn_reset.setText("Clean Selection")
        else:
            self.continue_selecting = True
            self.btn_reset.setText("Stop Painting Mode")
    
    def callback_cell_pick(self, unstructuredg_rid):
        if unstructuredg_rid is None:
            print("No cell was picked.")
            return

        cell_ids = unstructuredg_rid.cell_data.get("vtkOriginalCellIds")
        if cell_ids is None:
            print("No Cell IDs available in the selection.")
            return

        new_selected_cells = set(cell_ids.tolist()) 
        if self.continue_selecting:
            self.selected_cells = list(set(self.selected_cells).union(new_selected_cells))
        else:
            self.selected_cells = list(new_selected_cells)

        filtered_mesh = self.mesh.extract_cells(self.selected_cells)
        surface_mesh = filtered_mesh.extract_surface()

        self.view.plotter.add_mesh(surface_mesh, color="blue", show_edges=True)

        self.update_region_data(surface_mesh, unstructuredg_rid)

    def update_region_data(self, surface_mesh, unstructuredg_rid):
        unique_point = surface_mesh.points
        polygons = surface_mesh.faces.reshape((-1, 4))[:, 1:4]  # Assuming triangles
        # temperatures = surface_mesh["brightness_temperature"] # I should cinpare for different ?? 
        self.points_stack.append(unique_point) 

        self.all_points = np.vstack(self.points_stack) if self.points_stack else np.array([])

        
        self.polygons_stack.append(polygons)
        self.all_polygons = np.vstack(self.polygons_stack) if self.polygons_stack else np.array([])
        #print("ALL POLYGONS ",len(self.all_polygons),"POLYGONS INITiAL: ", len(polygons))
        

        self.rigion_type.polygons = self.all_polygons
        self.rigion_type.unstructuredg_rid = unstructuredg_rid
        self.rigion_type.points = self.all_points

        self.selected_cells_stack.append(self.selected_cells)
        self.all_selected_cells = np.concatenate(self.selected_cells_stack) if self.selected_cells_stack else np.array([])
        self.rigion_type.cell_ids = self.all_selected_cells


        self.rigion_type.rigion_name = "Test rigion"
        self.rigion_type.model_type =  self.model_type_combo.currentText()
        self.rigion_type.model_size = self.model_size_combo.currentText() 
        self.rigion_type.rigion_date = self.date_time_edit.dateTime().toString("yyyy-MM-dd HH:mm:ss") 

        self.temperatures_stack.append(surface_mesh["brightness_temperature"])
        self.all_temperatures = np.concatenate(self.temperatures_stack) if self.temperatures_stack else np.array([])
        self.rigion_type.brightness_temperature = self.all_temperatures

        self.latitude_stack.append(surface_mesh["latitude"])
        self.all_latitudes = np.concatenate(self.latitude_stack) if self.latitude_stack else np.array([])

        self.longitude_stack.append(surface_mesh["longitude"])
        self.all_longitudes = np.concatenate(self.longitude_stack) if self.longitude_stack else np.array([])
        self.rigion_type.longitude = self.all_longitudes
        self.rigion_type.latitude = self.all_latitudes

        self.incidence_angle_stack.append(unstructuredg_rid["incidence_angle"])
        self.all_incidence_angles = np.concatenate(self.incidence_angle_stack) if self.incidence_angle_stack else np.array([])
        self.rigion_type.incidence_angle = self.all_incidence_angles

        self.phase_angle_stack.append(unstructuredg_rid["phase_angle"])
        self.all_phase_angles = np.concatenate(self.phase_angle_stack) if self.phase_angle_stack else np.array([])
        self.rigion_type.phase_angle = self.all_phase_angles

        self.emission_angle_stack.append(unstructuredg_rid["emission_angle"])
        self.all_emission_angles = np.concatenate(self.emission_angle_stack) if self.emission_angle_stack else np.array([])
        self.rigion_type.emission_angle = self.all_emission_angles

        self.local_solar_stack.append(unstructuredg_rid["local_solar_time"])
        self.all_local_solar = np.concatenate(self.local_solar_stack) if self.local_solar_stack else np.array([])
        self.rigion_type.local_solar = self.all_local_solar

        self.pixel_x_stack.append(unstructuredg_rid["pixel_x"])
        self.all_pixel_x = np.concatenate(self.pixel_x_stack) if self.pixel_x_stack else np.array([])
        self.rigion_type.pixel_x = self.all_pixel_x

        self.pixel_y_stack.append(unstructuredg_rid["pixel_y"])
        self.all_pixel_y = np.concatenate(self.pixel_y_stack) if self.pixel_y_stack else np.array([])
        self.rigion_type.pixel_y = self.all_pixel_y

        self.local_solar_time_normal_vector_stack.append(unstructuredg_rid["local_solar_time_normal_vector"])
        self.all_local_solar_time_normal_vectors = np.concatenate(self.local_solar_time_normal_vector_stack) if self.local_solar_time_normal_vector_stack else np.array([])
        self.rigion_type.local_solar_time_normal_vector = self.all_local_solar_time_normal_vectors

        self.push_data(
            cell_ids=self.rigion_type.cell_ids,
            brightness_temperature=self.rigion_type.brightness_temperature,
            emission_angle=self.rigion_type.emission_angle,
            incidence_angle=self.rigion_type.incidence_angle,
            phase_angle=self.rigion_type.phase_angle,
            local_solar_time=self.rigion_type.local_solar,
            local_solar_time_normal_vector=self.rigion_type.local_solar_time_normal_vector,
            pixel_x=self.rigion_type.pixel_x,
            pixel_y=self.rigion_type.pixel_y,
            longitude=self.rigion_type.longitude,
            latitude=self.rigion_type.latitude,
            points=self.rigion_type.points,
            polygons=self.rigion_type.polygons,
        )
      
    def update_visulization(self):
        print(self.combo_visualization.currentText())

    def plot(self):
        
        self.ax.cla()

        visualization_type = self.combo_visualization.currentText()
        if visualization_type == "longitude/latitude":
            self.data_x = self.rigion_type.longitude
            self.data_y = self.rigion_type.latitude
        else:
            self.data_x = self.rigion_type.pixel_x
            self.data_y = self.rigion_type.pixel_y

        selected_scalar = self.combo_scalar_data.currentText()
        if selected_scalar == "brightness_temperature":
            temp = self.rigion_type.brightness_temperature
            label_txt = "Brightness Temperature"
        elif selected_scalar == "phase_angle":
            temp = self.rigion_type.phase_angle
            label_txt = "Phase Angle"
        elif selected_scalar == "incidence_angle":
            temp = self.rigion_type.incidence_angle
            label_txt = "Incidence Angle"
        elif selected_scalar == "emission_angle":
            temp = self.rigion_type.emission_angle
            label_txt = "Emission Angle"
        elif selected_scalar == "local_solar_time":
            temp = self.rigion_type.local_solar
            label_txt = "Local Solar Time"
        elif selected_scalar == "local_solar_time_normal_vector":
            temp = self.rigion_type.local_solar_time_normal_vector
            label_txt = "Local Solar Time Normal Vector"
        else:
            temp = self.rigion_type.brightness_temperature
            label_txt = "Brightness Temperature"
        
        self.data_vals = temp
        if temp is None or len(temp) == 0:
            print("No data available to plot.")
            return
        if self.colorbar is not None:
            try:
                self.colorbar.remove()
            except:
                pass
            self.colorbar = None

        if visualization_type == "longitude/latitude":
            valid_lons = self.rigion_type.longitude[self.rigion_type.longitude > 0]
            valid_lats = self.rigion_type.latitude[self.rigion_type.latitude > 0]
            self.ax.set_xlim(valid_lons.min(), valid_lons.max())
            self.ax.set_ylim(valid_lats.min(), valid_lats.max())
        else:  
            valid_pixels_x = self.rigion_type.pixel_x[self.rigion_type.pixel_x > 0]
            valid_pixels_y = self.rigion_type.pixel_y[self.rigion_type.pixel_y > 0]
            self.ax.set_xlim(valid_pixels_x.min(), valid_pixels_x.max())
            self.ax.set_ylim(valid_pixels_y.min(), valid_pixels_y.max())
        
        vmin = temp.min()
        vmax = temp.max()
        scatter = self.ax.scatter(
            self.data_x,
            self.data_y,
            c=temp,
            cmap='turbo',
            s=10,
            vmin=vmin,
            vmax=vmax
        )
        self.colorbar = self.fig.colorbar(scatter, ax=self.ax, shrink=0.6, label=label_txt)
        if visualization_type == "longitude/latitude":
            self.ax.set_title(f"{label_txt} on Asteroid Surface (Lat/Lon)")
            self.ax.set_xlabel("Longitude (°)")
            self.ax.set_ylabel("Latitude (°)")
        else:
            self.ax.set_title(f"{label_txt} on Asteroid Surface (Pixel X/Y)")
            self.ax.set_xlabel("Pixel (X) coordinates")
            self.ax.set_ylabel("Pixel (Y) coordinates")

        self.ax.grid()
        if self.rectangle_selector:
            self.rectangle_selector.disconnect_events()
        self.rectangle_selector = RectangleSelector(
            self.ax,
            self.onselect_rectangle, 
            useblit=True,
            button=[1],
            interactive=True,
            props=dict(
                facecolor='red',
                edgecolor='black',
                alpha=0.3,
                fill=True
            )
        )

        self.canvas.draw()
        self.push_data(plot_x=self.data_x, plot_y=self.data_y, plot_vals=self.data_vals)

   
    def onselect_rectangle(self, eclick, erelease):

        x_min, x_max = sorted([eclick.xdata, erelease.xdata])
        y_min, y_max = sorted([eclick.ydata, erelease.ydata])

        mask = (
            (self.data_x >= x_min) & (self.data_x <= x_max) &
            (self.data_y >= y_min) & (self.data_y <= y_max)
        )
        selected_indices = np.where(mask)[0]

        self.ax.scatter(self.data_x[mask], self.data_y[mask],
                        facecolor='none', edgecolor='red', s=40)
        self.canvas.draw()


    #--------------END------------#
    #------------UPDATE-PLOT------#
    def update(self):
        self.plot()
    #------------END--------------#
    #---------DOWNLAOD-VTK---------#
    def download_vtk(self):
        vtk_filename = 'filtered_mesh.vtk'
        self.rigion_type.write_vtk_polydata(vtk_filename)

    #-------------END--------------#



    #-----------SEND-DATA----------#
    def send_date(self):
        datetime = self.date_time_edit.dateTime()
        year = datetime.date().year()
        month = datetime.date().month()
        day = datetime.date().day()
        hour = datetime.time().hour()
        minute = datetime.time().minute()
        second = datetime.time().second()

        model = self.model_type_combo.currentText()
        model_size = self.model_size_combo.currentText()

        file = db_instance.get_name(year, month, day, hour, minute, second, model, model_size)
        
        print(file)
        if file:
            self.mesh = pv.read(file[0])
            print(self.mesh)
            self.setupVisualizationType()
            
    def setupVisualizationType(self):
        if self.mesh:
            visual = self.mesh[0].array_names
            print(visual)
            exclude = ['longitude', 'latitude', 'pixel_x', 'pixel_y']
            self.scalar_type_combo.clear()  
            for types in visual:
                if types not in exclude:
                    self.scalar_type_combo.addItem(types)

    def updateVisualizationType(self):

        selected_visual_type = self.scalar_type_combo.currentText()

        if self.mesh and selected_visual_type:

            self.view.plotter.clear()
            self.view.plotter.add_mesh(self.mesh, scalars=self.scalar_type_combo.currentText())
            self.view.plotter.update()
    #--------------END-------------#

    #-------------MODEL------------#
    def update_model_size(self):
        self.model_size_combo.clear()
        if self.model_type_combo.currentText() == "sfm":
            self.model_size_combo.addItems(["50k", "200k", "800k"])
        elif self.model_type_combo.currentText() == "spc":
            self.model_size_combo.addItems(["49k", "200k", "800k"])
    #--------------END-------------#

    #----------VALID-DATES---------#
    def set_valid_dates(self, ):
        dates_complex= db_instance.get_dates()
        dates_simple = db_instance.get_dates_day()
        dates =[]
        dates2 = []
        for d in dates_complex:
            parsed_date = datetime.strptime(d, '%Y-%m-%d %H:%M:%S')

            dates.append(parsed_date)
        
        for d in dates_simple:
            parsed_date = datetime.strptime(d, '%Y-%m-%d')

            dates2.append(parsed_date)
        return dates_complex, dates2
    
    def check_date(self):
        selected_date = self.date_time_edit.dateTime()
        selected_date_str = selected_date.toString("yyyy-MM-dd HH:mm:ss")

        allowed_dates = self.allowed_dates
        allowed_dates_qdatetime = [QDateTime.fromString(d, "yyyy-MM-dd HH:mm:ss") for d in allowed_dates]

        if selected_date not in allowed_dates_qdatetime:
            nearest_date = min(allowed_dates_qdatetime, key=lambda d: abs(selected_date.secsTo(d)))
            #print(f"Nearest allowed date for view {idx}: {nearest_date.toString('yyyy-MM-dd HH:mm:ss')}")
            
            self.date_time_edit.setDateTime(nearest_date) 
    #------------END--------------#

    def load_css(self, file_path):
        try:
            with open(file_path, 'r') as css_file:
                css = css_file.read()
                self.setStyleSheet(css)  # Apply the CSS styles
        except FileNotFoundError:
            print(f"Error: The file {file_path} was not found.")
        except Exception as e:
            print(f"Error loading CSS: {e}")   
    