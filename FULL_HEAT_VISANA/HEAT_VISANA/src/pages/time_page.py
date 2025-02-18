from PyQt6.QtWidgets import QLabel, QGridLayout, QSlider, QSizePolicy, QPushButton, QDateTimeEdit, QWidget,QComboBox, QVBoxLayout,QHBoxLayout
from PyQt6.QtCore import Qt,QDateTime,QTimer
from pyvistaqt import BackgroundPlotter
from datetime import datetime
import pyvista as pv
import os


from functools import partial
import sys

module_path = 'Application_code'
if module_path not in sys.path:
    sys.path.append(module_path)
db_path  = "Application_code/db/db.py"

if not os.path.exists(db_path):
    raise FileNotFoundError(f"Error: 'db.py' not found at {db_path}")
from db.db import DataBase 
db_instance = DataBase()


class PyVistaWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.layout = QVBoxLayout(self)
        self.setLayout(self.layout)

        #=----=
        self.plotter = BackgroundPlotter(show=False)  # This sets up the QVTKRenderWindowInteractor for you
        self.plotter.view_vector([1, 1, 1])
        self.layout.addWidget(self.plotter.interactor)
        
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)

    def update_plot(self, mesh, scalar):
        """Update the PyVista plot."""
        self.plotter.clear()  # Clear previous plot
        if mesh is not None and scalar in mesh.array_names:
            self.plotter.add_mesh(mesh, scalars=scalar, show_scalar_bar=True)
        else:
            print(f"Warning: Scalar '{scalar}' not found in mesh arrays.")
        self.plotter.update()

           

class TimePage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.initUI()
        self.views = []  # view references store
        #self.file_date_cache = {}
        
             
    def initUI(self):
        self.load_css('Application_code/src/style/multiview.css') 
        print(os.path.abspath('src/style/multiview.css'))
        

        layout = QVBoxLayout(self)
        top_layout = QVBoxLayout()
        self.selectView = QComboBox(self)
        self.selectView.addItem("How many views?")
        self.selectView.addItems(str(i) for i in range(1, 10))
        self.selectView.setFixedWidth(150)
        self.selectView.currentIndexChanged.connect(self.selectionchange)
        top_layout.addWidget(self.selectView, alignment=Qt.AlignmentFlag.AlignHCenter)

        self.plotter_widget = QWidget(self)
        self.grid_layout = QGridLayout(self.plotter_widget)
        self.plotter_widget.setLayout(self.grid_layout)

        layout.addLayout(top_layout)
        layout.addWidget(self.plotter_widget)
        self.setLayout(layout)
        self.file = ""

        #self.show()

    #----------CREATE-VIEW -----------#

    def selectionchange(self, i):
        if i > 0: 
            num_views = int(self.selectView.currentText())
            self.update_views(num_views)
    
    def get_grid_dimensions(self, num):
        if num <= 3:
            return 1, num
        elif num <= 6:
            return 2, (num + 1) // 2
        elif num <= 9:
            return 3, 3
        else:
            raise ValueError("Unsupported number of views")
        
    def update_views(self, num):
          
        # Clear previous layout content
        for i in reversed(range(self.grid_layout.count())):
            widget = self.grid_layout.itemAt(i).widget()
            if widget:
                widget.deleteLater()

        # Clear the views list
        self.views.clear()
        rows, cols = self.get_grid_dimensions(num)
        for i in range(self.grid_layout.rowCount()):
            self.grid_layout.setRowStretch(i, 0)
        for i in range(self.grid_layout.columnCount()):
            self.grid_layout.setColumnStretch(i, 0)
        for row in range(rows):
            self.grid_layout.setRowStretch(row, 1)  # Make rows expandable
        for col in range(cols):
            self.grid_layout.setColumnStretch(col, 1)
                                              
        for row in range(rows):
            for col in range(cols):
                index = row*cols +col    
                if index < num:
                    vtk_data_cache = {}
                    container = QWidget()
                    container_layout = QVBoxLayout(container)
                    container_layout.setContentsMargins(0, 0, 0, 0)

                    header_layout = QHBoxLayout()
                    date_time_edit_ini = QDateTimeEdit(QDateTime.currentDateTime())
                    date_time_edit_end = QDateTimeEdit(QDateTime.currentDateTime())

                    middle_layout = QHBoxLayout()
                    send_date = QPushButton('Set Date',self)
                    choose_model = QComboBox(self)
                    choose_model.addItems(["sfm", "spc"])
                    choose_model_size = QComboBox(self)
                    choose_model_size.addItems(["50k", "200k", "800k"])
                    choose_model.currentIndexChanged.connect(lambda _, idx=index: self.update_model_size(idx))
                                           
                    start_stop_animation = QPushButton('Star',self)
                    chooseVisualization = QComboBox(self)
                    
                    label_time = QLabel('Time: ')
                    #label_time.setObjectName("time_label")
                    bottom_layout = QVBoxLayout() #self ??
                    slider = QSlider(Qt.Horizontal)
                    slider.setMinimum(0)
                    slider.setMaximum(100) #need change to get len(files)
                    

                    all_dates, range_dates = self.set_valid_dates(date_time_edit_ini)
                    #Bottom data is it needed?
                    #all_dates_end, range_dates_end = self.set_valid_dates(date_time_edit_end) 
                    min_date, max_date = range_dates[0], range_dates[-1]

                    date_time_edit_ini.setDisplayFormat('yyyy-MM-dd HH:mm')
                    date_time_edit_ini.setCalendarPopup(True)
                    date_time_edit_ini.setMinimumDate(min_date)
                    date_time_edit_ini.setMaximumDate(max_date)

                    date_time_edit_end.setDisplayFormat('yyyy-MM-dd HH:mm')
                    date_time_edit_end.setCalendarPopup(True)
                    date_time_edit_end.setMinimumDate(min_date)
                    date_time_edit_end.setMaximumDate(max_date)

                    date_time_edit_ini.dateTimeChanged.connect(
                        lambda _, idx = index, edit=date_time_edit_ini: self.check_date(idx, edit))
                    date_time_edit_end.dateTimeChanged.connect(
                        lambda _, idx = index, edit=date_time_edit_end: self.check_date(idx, edit))

                    header_layout.addWidget(date_time_edit_ini)
                    header_layout.addWidget(date_time_edit_end)

                    middle_layout.addWidget(choose_model)
                    middle_layout.addWidget(choose_model_size)
                    middle_layout.addWidget(send_date)
                    middle_layout.addWidget(start_stop_animation)
                    middle_layout.addWidget(chooseVisualization)
                    middle_layout.addWidget(label_time)
                    
                    bottom_layout.addWidget(slider)

                    container_layout.addLayout(header_layout)
                    container_layout.addLayout(middle_layout)
                    container_layout.addLayout(bottom_layout)

                    animation_timer = QTimer()
                    
                    view = PyVistaWidget()
                    container_layout.addWidget(view)
                    self.views.append({
                        'plot_widget': view,
                        'date_time_edit':date_time_edit_ini,
                        'date_time_edit_end':date_time_edit_end,
                        'choose_model':choose_model,
                        'choose_model_size':choose_model_size,
                        'send_date': send_date,
                        'start_stop_animation':start_stop_animation,
                        'chooseVisualization':chooseVisualization,
                        'label_time': label_time,
                        'slider': slider,
                        'allowed_dates':all_dates,
                        'mesh':None,
                        "meshes":[],
                        "animating":False,
                        "current_mesh_index":0,
                        "vtk_data_cache": {},
                        "animation_timer":animation_timer

                    })
                    lambda _, idx=index: self.update_model_size(idx)
                    self.grid_layout.addWidget(container, row, col)
                    
                    #chooseVisualization.currentIndexChanged.connect(lambda value, idx=index: self.updateVisualizationType(idx, value))
                    chooseVisualization.currentIndexChanged.connect(partial(self.updateVisualizationType, index))

                    #this is passing the inde of visulization type but also I need to pass the view index?
                    
                    send_date.clicked.connect(lambda _, idx=index: self.load_meshes(idx, 0))
                    animation_timer.timeout.connect(lambda idx=index: self.animate_mesh(idx))
                    start_stop_animation.clicked.connect(lambda _, idx=index: self.toggle_animation(idx))
                    slider.valueChanged.connect(lambda value, idx=index: self.slider_changed(idx, value))
                         
    #-------------END-----------------#

    #------------LOAD-FILES-----------#
    
    def load_meshes(self, view_index, index):
         
        view = self.views[view_index]
        
        # Get the selected datetime range and model options
        datetime_ini = view['date_time_edit'].dateTime()
        datetime_end = view['date_time_edit_end'].dateTime()
        model = view['choose_model'].currentText()
        model_size = view['choose_model_size'].currentText()

        # Fetch the file names within the date range from the database
        self.files = db_instance.get_names(
            datetime_ini.date().year(), datetime_ini.date().month(), datetime_ini.date().day(),
            datetime_ini.time().hour(), datetime_ini.time().minute(), datetime_ini.time().second(),
            datetime_end.date().year(), datetime_end.date().month(), datetime_end.date().day(),
            datetime_end.time().hour(), datetime_end.time().minute(), datetime_end.time().second(),
            model, model_size
        )
        view["meshes"] = self.files  # Store file paths in view
        view["current_mesh_index"] = index

        # Cache meshes if not already cached
       
       #self.vtk_data_cache = {}  # Reset cache
        for filepath in self.files:
            if filepath not in  view["vtk_data_cache"]:
                mesh = pv.read(filepath)  # Read file only once
                if "brightness_temperature" in mesh.array_names:
                    view["vtk_data_cache"][filepath] = mesh  # Cache mesh data
                else:
                    print(f"Warning: 'brightness_temperature' not found in {filepath}")

        # Set slider maximum to the number of cached files
        view["slider"].setMaximum(len(view["vtk_data_cache"]) - 1)

        # Load the initial mesh from cache
        initial_mesh =  view["vtk_data_cache"][self.files[index]]
        if view["mesh"] is not None:
            view["plot_widget"].plotter.remove_actor(view["mesh"])
        self.setupVisualizationType(view_index)
        
        view["mesh"] = view["plot_widget"].plotter.add_mesh(initial_mesh, scalars=view['chooseVisualization'].currentText())
        view["plot_widget"].plotter.update() 

    def one_load(self, view_index, next_value):
        view = self.views[view_index]
        view["current_mesh_index"] = next_value

        # Get the file path and mesh data from cache
        filepath = view["meshes"][next_value]
        mesh =  view["vtk_data_cache"].get(filepath)
        if not mesh:
            print(f"Error: Mesh for {filepath} not found in cache.")
            return

        # Update the view with the cached mesh data
        scalar_name = view['chooseVisualization'].currentText()
        if scalar_name:
            view["plot_widget"].update_plot(mesh, scalar_name)

    #--------------END----------------#

    #--------VISULIZATION-TYPE--------#

    def setupVisualizationType(self, view_index):
        view = self.views[view_index]
        # Ensure we are working with the correct combo box and mesh
        #chooseVisualization = view['chooseVisualization']#something bad
        current_mesh_index = view["current_mesh_index"]
        filepath = view["meshes"][current_mesh_index]
        mesh =  view["vtk_data_cache"].get(filepath)


        if not mesh:
            print(f"Error: Mesh for {filepath} not found in cache.")
            return
        
        # Update scalar options
        scalar_names = mesh.array_names
        exclude = ['longitude', 'latitude', 'pixel_x', 'pixel_y']
        filtered_scalar_names = [name for name in scalar_names if name not in exclude]
        if view['mesh']:
            view['plot_widget'].plotter.remove_actor(view['mesh'])
            
        view['chooseVisualization'].clear()
        view['chooseVisualization'].addItems(filtered_scalar_names)

    def updateVisualizationType(self, view_index):
        view = self.views[view_index]
        myit = iter(self.views)

        # print(next(myit))
        # print("#----------------------------------------------------------#")
        # print(next(myit))
        scalar_name = view['chooseVisualization'].currentText()
        print(f"[DEBUG] Updating view {view_index} with scalar '{scalar_name}'")
        if not scalar_name:
            print("No visualization type selected.")
            return

        filepath = view["meshes"][view["current_mesh_index"]]
        mesh = view["vtk_data_cache"].get(filepath)
        if not mesh:
            print(f"Error: Mesh for {filepath} not found in cache.")
            return

        # Use update_plot method
        view["plot_widget"].update_plot(mesh, scalar_name)

    #--------------END----------------#

    #------------ANIMATION------------#
    def toggle_animation(self, index):
        view = self.views[index]
        if view["animating"]:
            view["animation_timer"].stop()  # Stop the animation
            view["start_stop_animation"].setText('Start Animation')  # Update button text
        else:
            view["animation_timer"].start(100)  # Change mesh every 250 milliseconds
            view["start_stop_animation"].setText('Stop Animation')  # Update button text
        view["animating"] = not view["animating"]

    def animate_mesh(self, view_index):
        view = self.views[view_index]
        self.updateVisualizationType(view_index)
        current_value = view["slider"].value()
        next_value = (current_value + 1) % len(view["meshes"])
        view["slider"].setValue(next_value)
          
    #--------------END----------------#
    
    #----------SLIDER-CHANGE----------#

    def slider_changed(self, view_index, value):
        view = self.views[view_index]
        
        if view["current_mesh_index"] != value:  # Avoid redundant updates
            self.one_load(view_index, value)

        # Extract and display time for the current slider value
        try:
            file_parts = view["meshes"][value].split("/")
            print(file_parts)
            print("Len: ",len(file_parts))
            date_time_str = file_parts[-1].split("_")  # assuming consistent file format
            print(date_time_str)
            # Parse date and time
            year, month, day = date_time_str[2][:4], date_time_str[2][4:6], date_time_str[2][6:]
            hour, minute, second = date_time_str[3][:2], date_time_str[3][2:4], date_time_str[3][4:]
            formatted_time = f"Time: {year}/{month}/{day} {hour}:{minute}:{second}"
            print(formatted_time)

            if view["label_time"].text() != formatted_time:
                view["label_time"].setText(formatted_time)

    
        except IndexError:
            print("Error: Filename format is unexpected.")

    #--------------END----------------#

    #----------VALID-DATES------------# 

    def set_valid_dates(self, date_time_edit):
         
        dates_complex = db_instance.get_dates()
        dates_simple = db_instance.get_dates_day()
        print()
        dates = []
        dates2 = []
        for d in dates_complex:
            parsed_date = datetime.strptime(d, '%Y-%m-%d %H:%M:%S')
            dates.append(parsed_date)
        for d in dates_simple:
            parsed_date = datetime.strptime(d, '%Y-%m-%d')
            dates2.append(parsed_date)
         
         
        return dates_complex, dates2
    
    def check_date(self, idx, date_time_edit):
        selected_date = date_time_edit.dateTime()
        selected_date_str = selected_date.toString("yyyy-MM-dd HH:mm:ss")
        
        allowed_dates = self.views[idx]["allowed_dates"]
        allowed_dates_qdatetime = [QDateTime.fromString(date,"yyyy-MM-dd HH:mm:ss" )for date in allowed_dates]

        if selected_date not in allowed_dates_qdatetime:
            neatrest_date = min(allowed_dates_qdatetime, key= lambda d: abs(selected_date.secsTo(d)))
            date_time_edit.setDateTime(neatrest_date)
    
    #--------------END----------------#

    #-----------MODEl-SIZE------------#
    def update_model_size(self, index):
        view = self.views[index]
        view["choose_model_size"].clear()
        if view["choose_model"].currentText() == "sfm":
            view["choose_model_size"].addItems(["50k", "200k", "800k"])
        elif view["choose_model"].currentText() == "spc":
            view["choose_model_size"].addItems(["49k", "200k", "800k"])
    #---------------END---------------#171
    
    #-----------LOAD-CSS--------------#
    def load_css(self, file_path):
        try:
            with open(file_path, 'r') as css_file:
                css = css_file.read()
                self.setStyleSheet(css)  # Apply the CSS styles
        except FileNotFoundError:
            print(f"Error: The file {file_path} was not found.")
        except Exception as e:
            print(f"Error loading CSS: {e}")
    #--------------END----------------# 
