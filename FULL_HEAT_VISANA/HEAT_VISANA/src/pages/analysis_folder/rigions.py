import sys
from PyQt6 import QtWidgets, QtCore
from PyQt6.QtWidgets import QLabel, QFormLayout, QWidget,QHBoxLayout, QFileDialog
from PyQt6.QtCore import Qt
import pyvista as pv
from pyvistaqt import QtInteractor
import numpy as np
import os
from db.db import DataBase 
db_instance = DataBase()
dbInsert = db_instance.insert_new_rigion

filename = "rigions_analysis_functions.py"

search_paths = [
    "Application_code/src/pages/analysis_folder",

]

file_found = False

for path in search_paths:
    full_path = os.path.join(path, filename)
    if os.path.isfile(full_path):
        print(f"File found at: {full_path}")
        file_found = True
        break

if not file_found:
    print("File not found in the specified search paths.")

from .rigions_analysis_functions import (
    MeshLoader, 
    MeshAnalyzer, 
    MeshVisualizer
)
class Regions(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.load_css('Application_code/src/style/analysis.css') 
        print(os.path.abspath('src/style/analysis.css'))
        
        

        self.central_widget = QtWidgets.QWidget()
        main_layout = QtWidgets.QHBoxLayout(self)
        left_layout = QtWidgets.QVBoxLayout()

        controls_layout = QtWidgets.QVBoxLayout()
        
        self.points_spinbox = QtWidgets.QSpinBox()
        self.points_spinbox.setMinimum(1)
        self.points_spinbox.setMaximum(20)
        self.points_spinbox.setValue(6)
        self.points_spinbox.setFixedWidth(50)

        points_layout = QtWidgets.QHBoxLayout()

        points_layout.addWidget(QtWidgets.QLabel("Number of points to pick:"))
        points_layout.addWidget(self.points_spinbox)
        
        points_layout.addStretch()

        self.point_counter_label = QtWidgets.QLabel("Points Picked: 0 / " + str(self.points_spinbox.value()))
        self.point_counter_label.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        self.point_counter_label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)

        points_layout.addWidget(self.point_counter_label, alignment=Qt.AlignLeft)


        load_pick_layout = QtWidgets.QHBoxLayout()

        self.load_button = QtWidgets.QPushButton("Load Mesh")
        self.load_button.clicked.connect(self.load_mesh)
        load_pick_layout.addWidget(self.load_button)
        
        self.pick_button = QtWidgets.QPushButton("Enable Point Picking")
        self.pick_button.clicked.connect(self.enable_picking)
        load_pick_layout.addWidget(self.pick_button)

        ana_visu_layout = QtWidgets.QHBoxLayout()
        self.analysis_button = QtWidgets.QPushButton("Run Analysis")
        self.analysis_button.clicked.connect(self.run_analysis)
        ana_visu_layout.addWidget(self.analysis_button)
        
        self.visualize_button = QtWidgets.QPushButton("Visualize Angle")
        self.visualize_button.clicked.connect(self.visualize_angle)
        ana_visu_layout.addWidget(self.visualize_button)
        
        self.reset_button = QtWidgets.QPushButton("Reset Selection")

        self.reset_button.clicked.connect(self.reset_selection)

        

        controls_layout.addLayout(points_layout)
        controls_layout.addLayout(load_pick_layout)
        controls_layout.addLayout(ana_visu_layout)
        controls_layout.addWidget(self.reset_button)
        

        left_layout.addLayout(controls_layout)
        
        results_layout = QtWidgets.QHBoxLayout()

        self.results_frame = QtWidgets.QFrame()
        self.results_frame.setObjectName("resultsRow")

        self.results_layout = QtWidgets.QFormLayout(self.results_frame)
        self.results_layout.setLabelAlignment(Qt.AlignLeft)
        self.results_layout.setContentsMargins(0, 0, 0, 0) 
        
        self.depth_label = QtWidgets.QLabel("")
        self.angle_label = QtWidgets.QLabel("")
        self.slope_label = QtWidgets.QLabel("")
        self.geodesic_diameter_label = QtWidgets.QLabel("")
        self.geodesic_radius_label = QtWidgets.QLabel("")
        self.centroid_label = QtWidgets.QLabel("")
        self.crater_diameter_label = QtWidgets.QLabel("")

        depth_title = QLabel("Depth:")
        depth_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(depth_title, self.depth_label)
        depth_title.setObjectName("title")

        anlge_title = QLabel("Angle with Horizontal:")
        anlge_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(anlge_title, self.angle_label)
        anlge_title.setObjectName("title")

        slope_title = QLabel("Average Crater Slope:")
        slope_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(slope_title, self.slope_label)
        slope_title.setObjectName("title")

        geodesic_diameter_title = QLabel("Geodesic Diameter:")
        geodesic_diameter_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(geodesic_diameter_title, self.geodesic_diameter_label)
        geodesic_diameter_title.setObjectName("title")

        geodesic_radius_title = QLabel("Geodesic Radius:")
        geodesic_radius_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(geodesic_radius_title, self.geodesic_radius_label)
        geodesic_radius_title.setObjectName("title")

        centroid_title = QLabel("Centroid:")
        centroid_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(centroid_title, self.centroid_label)
        centroid_title.setObjectName("title")


        crater_diameter_title = QLabel("Crater Diameter from the Circle:")
        crater_diameter_title.setStyleSheet("font-weight: bold;")
        self.results_layout.addRow(crater_diameter_title, self.crater_diameter_label)
        crater_diameter_title.setObjectName("title")

        
        results_layout.addWidget(self.results_frame,  alignment=Qt.AlignLeft)
        left_layout.addLayout(results_layout)
        
        bottom_layout = QtWidgets.QHBoxLayout()
        self.add_db_button = QtWidgets.QPushButton("Add Region in to the db")
        self.add_db_button.clicked.connect(self.add_region_db)
        bottom_layout.addWidget(self.add_db_button)
        left_layout.addLayout(bottom_layout)

        right_layout = QtWidgets.QVBoxLayout()
        self.plotter_widget = QtInteractor(self.central_widget)
        right_layout.addWidget(self.plotter_widget.interactor)

        

        main_layout.addLayout(left_layout,1)
        main_layout.addLayout(right_layout,2)


        self.mesh_loader = None
        self.analyzer = None
        self.visualizer = None
        self.picked_points = []  
        self.analysis_results = {} 
        self.picked_point_actors = []

        self.pick_button.setEnabled(False)
        self.analysis_button.setEnabled(False)
        self.visualize_button.setEnabled(False)
        self.reset_button.setEnabled(False)
        

    def load_mesh(self):
        options = QFileDialog.Options()
        vtk_file_path, _ = QFileDialog.getOpenFileName(
            self, "Open VTK File", "", "VTK Files (*.vtk);;All Files (*)", options=options
        )
        if vtk_file_path:
            self.mesh_loader = MeshLoader(vtk_file_path)
            self.mesh_loader.load_and_align_mesh()
            self.analyzer = MeshAnalyzer(self.mesh_loader.mesh)
            self.visualizer = MeshVisualizer(self.mesh_loader, self.analyzer)
            MeshVisualizer.MAX_PICKS = self.points_spinbox.value()

            self.plotter_widget.add_mesh(
                self.mesh_loader.mesh,
                scalars="Z-Gradient",
                cmap="terrain",
                opacity=0.8,
                show_edges=True,
                reset_camera=True
            )
            self.plotter_widget.reset_camera()
            self.plotter_widget.render()

            self.pick_button.setEnabled(True)
            self.analysis_button.setEnabled(False)
            self.visualize_button.setEnabled(False)
            self.reset_button.setEnabled(False)
        else:
            print("No file selected!")

    def enable_picking(self):
        self.disable_current_picking()
        self.picked_points = []
        self.point_counter_label.setText("Points Picked: 0 / " + str(MeshVisualizer.MAX_PICKS))
        for actor in self.picked_point_actors:
            self.plotter_widget.remove_actor(actor, render = False)
        self.picked_point_actors = []
        self.plotter_widget.enable_point_picking(
            callback=self.point_picked,
            use_picker=True,
            show_message=True
        )
    def disable_current_picking(self):
        try:
            self.plotter_widget.disable_picking()
            self.picked_points = []
            self.point_counter_label.setText(f"Points Picked: {len(self.picked_points)} / {MeshVisualizer.MAX_PICKS}")
        except AttributeError:
            try:
                self.plotter_widget.plotter.disable_picking()
                self.picked_points = []
                self.point_counter_label.setText(f"Points Picked: {len(self.picked_points)} / {MeshVisualizer.MAX_PICKS}")

            except Exception as e:
                print("Error disabling picking:", e)
        except Exception as e:
            print("Error disabling picking:", e)

    def point_picked(self, point, index):
        self.picked_points.append(point)
        sphere = pv.Sphere(center=point, radius=0.001)
        actor = self.plotter_widget.add_mesh(sphere, color="red", reset_camera=False)
        self.picked_point_actors.append(actor)

        self.plotter_widget.render()
        self.point_counter_label.setText(f"Points Picked: {len(self.picked_points)} / {MeshVisualizer.MAX_PICKS}")

        if len(self.picked_points) == MeshVisualizer.MAX_PICKS:
            self.pick_button.setEnabled(False)
            self.analysis_button.setEnabled(True)
            self.reset_button.setEnabled(True)

    def run_analysis(self):
        if len(self.picked_points) != MeshVisualizer.MAX_PICKS:
            return
        try:
            results = self.visualizer.run_analysis_without_plotting(self.picked_points)
            
        except Exception as e:
            print("Error during analysis:", e)
            return
        
        self.analysis_results = results

        self.depth_label.setText(f"{results['Depth']*1000:.4f} m")  # adjust if necessary
        self.angle_label.setText(f"{results['Angle with Horizontal']:.2f} degrees")
        self.slope_label.setText(f"{results['Average Crater Slope']:.2f} degrees")
        self.geodesic_diameter_label.setText(f"{results['Geodesic Diameter']:.4f} m")
        self.geodesic_radius_label.setText(f"{results['Geodesic Radius']:.4f} m")
        centroid = results["Centroid (C)"]
        self.centroid_label.setText(f"{centroid}")
        self.crater_diameter_label.setText(f"{results['Crater Diameter from the Circle']:.4f} m")

        self.visualize_button.setEnabled(True)


    def visualize_angle(self):
        for actor in self.picked_point_actors:
            self.plotter_widget.remove_actor(actor, render = False)
        self.picked_point_actors = []
        if not self.analysis_results:
            print("No analysis results available. Please run analysis first.")
            return
        inside_faces = self.analysis_results.get("Inside Faces", None)
        if inside_faces is None:
            print("Inside faces not available in analysis results.")
            return

        def rotate_vector(vec, axis, angle_rad):
            axis = axis / np.linalg.norm(axis)
            cos_a = np.cos(angle_rad)
            sin_a = np.sin(angle_rad)
            dot_a = np.dot(axis, vec)
            cross_a = np.cross(axis, vec)
            return (vec * cos_a + cross_a * sin_a + axis * dot_a * (1 - cos_a))


        mesh = self.mesh_loader.mesh
        surface = mesh.extract_surface()

        selected_polydata = surface.extract_cells(list(inside_faces))
        region_points = selected_polydata.points

        C = self.analyzer.compute_centroid_of_cells(inside_faces)
        try:
            intersection_point = self.analyzer.find_vertical_intersection(C)
        except Exception as e:
            print(f"Vertical intersection error: {e}")
            return

        A_index = mesh.find_closest_point(intersection_point)
        A = mesh.points[A_index]
        B = region_points[np.argmax(region_points[:, 2])]
        depth = B[2] - A[2]
        delta_xyz = B - A
        delta_xy = np.linalg.norm(delta_xyz[:2])
        angle_deg = np.degrees(np.arctan2(delta_xyz[2], delta_xy)) if delta_xy > 1e-12 else 0.0
        average_slope = self.analyzer.compute_crater_slope(inside_faces, method='average')

        h = np.array([delta_xyz[0], delta_xyz[1], 0.0])  # horizontal projection
        v = delta_xyz
        norm_h = np.linalg.norm(h)
        norm_v = np.linalg.norm(v)
        arc_pts = []
        if norm_h > 1e-12 and norm_v > 1e-12:
            rot_axis = np.cross(h, v)
            axis_len = np.linalg.norm(rot_axis)
            if axis_len > 1e-12:
                rot_axis /= axis_len
                dot_val = np.dot(h, v)
                cos_val = dot_val / (norm_h * norm_v)
                cos_val = np.clip(cos_val, -1.0, 1.0)
                angle_between = np.arccos(cos_val)
                n_arc_points = 30
                for t in np.linspace(0, angle_between, n_arc_points):
                    rotated = rotate_vector(h, rot_axis, t)
                    arc_pts.append(A + rotated)
        arc_pts = np.array(arc_pts)

        self.plotter_widget.add_mesh(selected_polydata, color="green", opacity=0.5, reset_camera=False)
        AB_points = np.array([A, B])
        self.plotter_widget.add_points(AB_points, color="purple", point_size=15, render_points_as_spheres=True, reset_camera=False)
        line_AB = pv.Line(A, B)
        self.plotter_widget.add_mesh(line_AB, color="magenta", line_width=3, reset_camera=False)
        if arc_pts.size:
            arc_poly = pv.PolyData()
            arc_poly.points = arc_pts
            cells = []
            for i in range(len(arc_pts) - 1):
                cells.append([2, i, i + 1])
            arc_poly.lines = np.hstack(cells)
            self.plotter_widget.add_mesh(arc_poly, color="purple", line_width=3, reset_camera=False)
        text_str = f"Angle: {angle_deg:.2f}°\nDepth: {depth*1000:.4f} m\nAvg Slope: {average_slope:.2f}°"
        
        self.plotter_widget.render()

    def reset_selection(self):
        
        self.disable_current_picking()

        self.picked_points = []
        self.analysis_results = {}

        self.depth_label.setText("")
        self.angle_label.setText("")
        self.slope_label.setText("")
        self.geodesic_diameter_label.setText("")
        self.geodesic_radius_label.setText("")
        self.centroid_label.setText("")
        self.crater_diameter_label.setText("")

        self.plotter_widget.clear()
        # Re-add the base mesh
        self.plotter_widget.add_mesh(
            self.mesh_loader.mesh,
            scalars="Z-Gradient",
            cmap="terrain",
            opacity=0.8,
            show_edges=True,
            reset_camera=True
        )
        self.plotter_widget.reset_camera()
        self.plotter_widget.render()

        self.pick_button.setEnabled(True)
        self.analysis_button.setEnabled(False)
        self.visualize_button.setEnabled(False)
        self.reset_button.setEnabled(False)

    def add_region_db(self):
        region_name, ok = QtWidgets.QInputDialog.getText(
            self,
            "Add Region",
            "Enter region name: "
        )
        
        
        if ok and region_name.strip():
            print(region_name.strip())
            if not self.analysis_results:
                print("No analysis results available. Please run analysis first.")
                return
            results = self.analysis_results
            depth = results['Depth']*1000
            angle = results ['Angle with Horizontal']
            slope = results['Average Crater Slope']
            geo_diameter = results["Geodesic Diameter"]
            geo_radius = results["Geodesic Radius"]
            centroid = results["Centroid (C)"]
            crater_dimater = results["Crater Diameter from the Circle"]
            self.insert_region_into_db(region_name, depth, angle, slope, geo_diameter, geo_radius, centroid[0],centroid[1],centroid[2],crater_dimater)
        else:

            print("Cancel")
    
    def insert_region_into_db(self, region_name, depth, angle, slope, geo_diameter, geo_radius, centroid_x, centroid_y,centroid_z ,crater_dimater):
        dbInsert(region_name, float(depth), float(angle), float(slope), float(geo_diameter), float(geo_radius), float(centroid_x), float(centroid_y),float(centroid_z) ,float(crater_dimater))


    def load_css(self, file_path):
        try:
            with open(file_path, 'r') as css_file:
                css = css_file.read()
                self.setStyleSheet(css)
        except FileNotFoundError:
            print(f"Error: The file {file_path} was not found.")
        except Exception as e:
            print(f"Error loading CSS: {e}")
    #--------------END----------------# 