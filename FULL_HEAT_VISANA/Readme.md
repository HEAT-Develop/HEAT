
# HEAT\_VISANA Documentation

**HEAT\_VISANA** is a module from **HEAT** primarily developed by the HEAT development team: **VILARELL BELLÉS Ramon, HIROHIDE Demura, ARAI Takehiko**, with the invaluable support of **Hiroki Senshu, Naoya Sakatani, and Tatsuaki Okada**. 

**HEAT\_VISANA** is designed to integrate three core functions, each backed by a database, to facilitate asteroid visualization and analysis. Currently, **HEAT\_VISANA** is tested only on **macOS**. Users can navigate through its functions using a menu interface. The primary aim is to assist scientists with visualization and analysis, particularly in crater-related research. 

HEAT\_VISANA is designed as a module to visualize and analyze asteroid data, primarily focusing on crater identification, depth analysis, and surface temperature mapping. By integrating multiple data sources (e.g., mesh models, thermal infrared data), HEAT\_VISANA aims to streamline research workflows that investigate asteroid geology, surface processes, and thermal characteristics. 

This module is particularly valuable for: 

1. **Thermal Data Visualization:** Understanding surface temperature variations.
1. **Geodesic Measurements:** Supporting advanced geometry-based calculations (e.g., geodesic distance, crater depth) ,

By consolidating these capabilities in a single interface, HEAT\_VISANA reduces the complexity of running multiple, disjointed tools. 

Target Audience

- **Planetary Scientists and Researchers**: Specialists looking to explore asteroid surface features, crater statistics, and temporal changes in thermal data for academic or mission-related studies. 
- **Graduate Students**: Those in planetary science, astronomy, or geophysics programs who require a user-friendly yet robust toolkit for hands-on research, thesis work, or coursework projects.
- **Software Developers**: Individuals contributing to data visualization or scientific computing projects.  

By clearly defining the purpose, scope, and audience, readers of your documentation will have a better understanding of why HEAT\_VISANA exists, what unique value it adds to asteroid research, and who benefits most from its features. 

## Installation Guide

*Python3 Installation and HEAT\_VISANA Requirements* Ensure Python 3 is installed. Check the version with: 

    python3 --version 

If it's not installed, execute the following command: 

    brew install python3 

After installation, install the necessary libraries from the requirements.txt file: 

    pip3 install -r requirements.txt 

To verify pip installation: 

    pip3 --version 

If pip is missing, install it with: 

    python3 -m ensurepip --upgrade 

### Database Installation

1. **Verify MySQL Installation** 
- Open **System Settings** -> Select your **Mac User** -> Scroll down the left sidebar. 
- If you see **MySQL**, it's already installed. 
2. **Install MySQL (if not installed)** 
- Visit:[ MySQL Downloads ](https://dev.mysql.com/downloads/mysql/)https://dev.mysql.com/downloads/mysql/
- Select your macOS version and follow the installation instructions. 
- Restart your machine after installation.
3. **Install MySQL Workbench** 
- Visit:[ MySQL Workbench Downloads ](https://dev.mysql.com/downloads/workbench/)https://dev.mysql.com/downloads/workbench/
- Download and install the appropriate version for your macOS.
4. **Configure MySQL Workbench** 
- Open MySQL Workbench. 
- Next to **MySQL Connections**, click the **+** button.  
- **Connection Name**: Any name of your choice. 
- **Hostname**: localhost
- **Username**: root *(recommended)* 
- **Password**: *User-defined* 

**Note:** Using root is recommended for simplicity. If using another username, ensure appropriate permissions are granted. 

5. **Server Management Configuration** 
- Click **Configure Server Management**. 
- Proceed through the steps, ignoring errors like: 
- *Get Server Version: Unsupported Server Version* 
- *Get Server OS* 
- Press **Continue** until **Finish**. 
6. **Database Import** 
- Open the new connection. 
- Ignore **Connection Warning** if it appears. 
- Go to **Server** -> **Data Import**. 
- Select **Import from Dump Project Folder**. 
- Choose the  **Dump** folder from **HEAT\_VISANA**. 
- Click **Load Folder Content** -> **Start Import**. 
- Upon completion, confirm the **hyb2\_tir\_lv3** database appears under **Schemas**. 


### Running HEAT\_VISANA 

1. **Open with Visual Studio Code** 
- Open the **HEAT\_VISANA** project folder. 
- Go to the folder db. 
- Edit the db.py file. 

self.user = Your database
username self.password = Your database password 
self.host = Set if you did not use localhost

2. **Open Terminal** 
- Navigate to the  **HEAT\_VISANA** directory. 
- Run the application: 

python3 HEAT\_VISANA/main.py 

 1. **Open with Visual Studio Code** 
 2. Open the  **HEAT\_VISANA** project folder. 
 3. Open the integrated terminal. 
 4. Run the code 

**HEAT\_VISANA** should now be up and running, ready to support your scientific visualization and analysis tasks related to asteroid craters. 

## Module Navigation

The module interface contains several pages accessible from the menu:

 1. **Analysis Page**
 2. **Visulizaton over time Page**
 3. **Visulization one specific file Page**
 4. **Databse management and CSV -> VTK covnersion Page.**

### Analysis Page:

This page includes a sub-menu with:

* **Selection Page**
* **Regions Page** 

#### *Selection Page Features*

It is the first page that appears when the user opens HEAT\_VISANA This page has 16 features for the visulizaton and Anlaysis of the asteroids. 

2  3  4  5  11  12  13  14 !1 
6  7 
15 
8 
16 9  10 

1  **Date Selection:** Choose the infrared camera shot date. 
2  **Model Specification:** Choose between spc and sfm models.  3  **Model Size:** Adjust model size based on the specification.  
4  **Start model:** Plots the selected model from the database. !
5  **Scalar Selection:** Choose a scalar data to update the displayed  mesh.  
6  **Region Selection:** Select asteroid regions to view mean curvature. 
7  **Reset Selection:** Stops and resets the selected region. 
8  **Canvas Display:** Shows the selected asteroid view. 
9  **Download CSV:** Downloads selected data in CSV format. 
10  **Download VTK:** Exports the selected region as a VTK file. 
11  **Plot Selected Data:** Plots selected region data. 
12  **Scalar Selection:** Choose a scalar data to update de plot.  13  **Axis Selection:** Select x and f(x) axes for plotting.  
14  **Console Toggle:** Show/hide Python3-based console. 
15  **Data Plot Display:** Visualizes selected data. 
16  **Console Data:** Displays plotted data for analysis. 
**CSV File Format** 

- **region\_id:** Unique ID for each region. 
- **timestamp:** Data timestamp. 
- **latitude**, **longitude**, **brightness_temperature**, **phase\_angle**, **incidence\_angle**, **emission\_angle**, **local\_solar\_time**, **local\_solar\_time\_normal\_vector**: Data from the vtk file 
- **x, y, z**: Vertex coordinates. 
- **nx, ny, nz**: Vertex normal coordinates. 
- **poly\_id**: Polygon ID from the VTK file. 

#### *Regions Page Features* 

This page is used based on the VTK download from the Selection Page and aims to assist in crater analysis. 

1  2 
3  4  10 5  6 

7 

8 

9 
1  **Point Selection:** Select the number of points to pick in the region. 
2  **Point Count Display:** Displays the selected points relative to the maximum allowed. 
3  **Load Mesh Button:** Loads the mesh for analysis. 
4  **Enable Point Picking:** Activates the point-picking feature. 
5  **Run Analysis:** Initiates the mesh analysis. 
6  **Visualize Selection and Angle:** Displays the selected region and its angle. 
7  **Analysis Data Display:** Shows the analyzed data. 
8  **Reset Selection:** Resets the current selection. 
9  **Add Region to Database:** Saves the selected region's information. 
10  **Canvas Display:** Displays the mesh for visualization. !

**Regions Page Data Fields** 

- Depth: Depth of the crater. 
- Angle with Horizontal: Angle measurement. 
- Average Crater Slope: Slope calculation. 
- Geodesic Diameter: Diameter based on geodesic distance. 
- Geodesic Radius: Geodesic radius. 
- Centroid: Central point of the crater. 
- Crater Diameter from the Clock: Diameter measured based on clock positioning. 

### Visulizaton over time Page:

This page is used to visualize data over time, tracking changes experienced by the asteroid. 

1 
2  3 
4  5  6  7  9 
8 
10 

1  **Combo Box:** Select the number of views.   
2  **Start Date:** Choose the initial date for visualization.  
3  **End Date:** Choose the final date for visualization.  
4  **Model Specification:** Choose between spc and sfm models   **
5  **Model Size:** Adjust model size based on the specification. 
6  **Load Mesh Button:** Load the mesh.
7  **Visualize Data Over Time:** Button to animate data changes over time.** 
8  **Scalar Selection**: Choose a scalar data to update the displayed  
mesh.  
9  **Timestamp Display:** Displays the time corresponding to the current mesh view. 
10  **Canvas Display:** Displays the mesh for visualization. 

### Visulization one specific file Page:

This page is used to visualize specific data with spaicifc time shot. 

1 
2  3  4  5  6 
7 
1  **Combo Box:** Select the number of views.   
2  **Date:** Choose the specific date for visualization.  
3  **Model Specification:** Choose between spc and sfm models   **
4  **Model Size:** Adjust model size based on the specification.  5  **Load Mesh Button:** Load the mesh. 
6  **Scalar Selection**: Choose a scalar data to update the displayed mesh.  
7  **Visualize Data Over Time:** Button to animate data changes over time.**  

### Databse management and CSV -> VTK covnersion Page. 

This page is used to add the files to the databse and convert csv files to vtk files  1  2  3 
4 
5 
1  **Model Size:** Adjust model size based on the specification. 
2  **Browse Button:** Select the CSV file to convert. 

3  **Convert CSV to VTK Button:** Executes the file conversion. 

4  **Add to Database Button:** Adds the converted files to the database. 
5  **File List Display:** Lists the files added to the database. 

## Database Structure:

The database for HEAT\_VISANA is structured into three core tables: **Missions**, **Models**, and **Regions**. Here’s a detailed breakdown of each table: 

1. *Missions* 

   This table stores information about various asteroid missions and the corresponding 3D model files. 



|Field** |Data Type** |Description** |
| - | - | - |
|**id**|INTEGER|Unique identifier for each mission. |
|**name**|VARCHAR|Name of the mission file. |
|**level**|VARCHAR|Data processing level (e.g., L3). |
|**mission**|VARCHAR|Mission name (e.g., "hyb2"). |
|**camera**|VARCHAR|Camera type used (e.g., TIR for Thermal Infrared). |
|**year**|INTEGER|Year of data acquisition. |
|**month**|INTEGER|Month of data acquisition. |
|**day**|INTEGER|Day of data acquisition. |
|**hour**|INTEGER|Hour of data acquisition. |
|**min**|INTEGER|Minute of data acquisition. |
|**sec**|INTEGER|Second of data acquisition. |
|**format**|VARCHAR|File format (e.g., "vtk"). |
|**model**|VARCHAR|Model type used (e.g., "sfm" or "spc"). |
|**vtk\_file\_path**|VARCHAR|File path to the .vtk file on the system. |
|**model\_size**|VARCHAR|Model resolution/size (e.g., "200k"). |

2. ***Models*** 

   This table contains metadata about the 3D models of the asteroid. 



|Field** |Data Type** |Description** |
| - | - | - |
|**id**|INTEGER|Unique identifier for each model. |
|**name**|VARCHAR|Name of the model. |
|**asteroid**|VARCHAR|Name of the asteroid (e.g., Ryugu). |
|**type**|VARCHAR|File format type (e.g., "vtk"). |
|**specification**|VARCHAR|Model specification (e.g., "spc" or "sfm"). |
|**file\_path**|VARCHAR|File path to the model on the filesystem. |
|**size**|VARCHAR|Model resolution (e.g., "49k"). |

3. ***Regions*** 

   This table contains data about specific regions on the asteroid's surface, such as craters. 

|Field** |Data Type** |Description** |
| - | - | - |
|**id**|INTEGER|Unique identifier for each region. |
|**name**|VARCHAR|Descriptive name for the region (e.g., "Crater\_01"). |
|**depth**|FLOAT|Depth of the region (e.g., a crater) in meters. |
|**angle\_with\_horizontal**|FLOAT|Angle of the surface relative to the horizontal plane. |
|**avg\_crater\_slope**|FLOAT|Average slope of the crater's interior. |
|**geodesic\_diameter**|FLOAT|Diameter of the crater based on geodesic calculations. |
|**geodesic\_radius**|FLOAT|Radius corresponding to the geodesic diameter. |
|**centroid\_x**|FLOAT|X-coordinate of the region's centroid. |
|**centroid\_y**|FLOAT|Y-coordinate of the region's centroid. |
|**centroid\_z**|FLOAT|Z-coordinate of the region's centroid. |
|**crater\_diameter**|FLOAT|Estimated diameter of the crater. |

11 

## Folder Distribution 


### db.py

This file contains one main class responsible for database connectivity and management. Before using the class, you must configure the following variables: 

- self.user
- self.password
- self.hos

These variables are used to connect to the database. The class provides **18 functions** for managing database operations: 

1. **get\_year:** 

   Accepts a year value, connects to the MySQL database, and searches the missions table for records matching the provided year. Returns all matching records. 

2. **get\_month:** 

   Accepts a month value and queries the missions table for records where the month field equals the provided value. Returns the resulting data. 

3. **get\_day:** 

   Accepts a day value and queries the missions table for records where the day field equals the provided value. Returns the resulting data. 

4. **get\_minut:** 

   Accepts a minute value and queries the missions table for records where the minute field equals the provided value. Returns the resulting data. 

5. **get\_second:** 

   Accepts a second value and queries the missions table for records where the second field equals the provided value. Returns the resulting data. 

6. **get\_hour:** 

   Accepts an hour value and queries the missions table for records where the hour field equals the provided value. Returns the resulting data. 

7. **get\_name:** 

   Requires several inputs (year, month, day, hour, minute, second, doel, and model\_size). It creates a filename based on the timestamp values, then queries the missions table for the corresponding VTK file path that matches all provided parameters. Returns the fetched VTK file path. 

8. **get\_names:** 

   Accepts a time range (initial and end values for year, month, day, hour, minute, and second) along with model and model\_size parameters. It queries the missions table for records falling within the specified time range and matching the model details. Returns a list of VTK file paths. 

9. **get\_vtk\_file\_paths:** 

   Queries the missions table for records where the model is "sfm" and the model size is "50k", then extracts and returns all corresponding VTK file paths as a list. 

10. **get\_all\_names:** 

    Accepts model and model\_size as inputs, fetches all VTK file paths from the missions table that match these criteria, and returns the results as a list. 

11. **insert\_new\_mission:** 

    Takes various inputs related to a mission record (name, level, mission details, camera, timestamp, format, model, VTK file path, and model size). It checks if a record with the same timestamp and model details exists; if no duplicate is found, it inserts the new record into the database. 

12. **get\_dates:** 

    Retrieves all distinct date\_time strings from the missions table (formatted as “YYYY-MM-DD HH:MM:SS”), orders them chronologically, and returns them as a list. 

13. **get\_dates\_day:** 

    Similar to get\_dates but returns only the date portion (“YYYY-MM-DD”) without the time. 

14. **get\_full\_day:** 

    Accepts a specific year, month, and day, then queries the missions table for all distinct VTK file paths recorded on that day. Returns a list of file paths. 

15. **insert\_new\_model:** 

    Accepts parameters for a model record (name, asteroid, type, specification file path, and size) and inserts a new record into the models table. 

16. **model\_in\_db:** 

    Accepts a model name and checks the models table to determine if a model with the same name already exists. Returns a boolean value. 

17. **get\_model:** 

    Accepts a size parameter and queries the models table to fetch the file path of a model that matches the given size. Returns the file path if found. 

18. **insert\_new\_rigion:** 

    Takes parameters such as region name, depth, angle, slope, geodesic diameter, geodesic radius, centroid coordinates, and crater diameter, then inserts a new region record into the regions table. 

### create\_a\_new\_csv.py 

This file contains one function: 

1\.  **combine\_vtk\_csv:** 

Processes a CSV file containing VTK data to ensure all expected polygon IDs are represented in the output file. It reads the CSV into a DataFrame, identifies missing polygon IDs based on the provided model name (e.g., 49k, 200k, or 800k), creates new rows with default values for each missing ID, concatenates these rows with the original data, sorts by polygon ID, and writes the updated DataFrame to a new CSV file. 

### csv\_to\_vtk.py 

This file contains two functions: 

1. **extract\_and\_write\_vtk:** 

   Reads a VTK file using PyVista, extracts the mesh’s points and polygonal faces, and writes a new CTK file including both geometric data and additional cell data fields. It calculates the number of points and polygons, writes a header line indicating the model type, writes the points and polygons, and appends cell data for each field. 

2. **combine\_vtk\_csv:** 

   Integrates information from a VTK file and a CSV file. It reads the VTK file to obtain mesh information, reads the CSV file into a DataFrame to retrieve corresponding cell data, extracts required columns into a dictionary (cell\_data), and calls extract\_and\_write\_vtk with the original VTK filename, the desired output 

   filename, and the assembled cell data. 

### upload\_page.py 

This file contains several functions: 

1. **\_\_init\_\_:** 

   Initializes the UploadPage widget by setting the title, building the user interface (via initUI()), and ensuring the required model files are present in the database by calling model\_in\_dataBase(). 

2. **initUI:** 

   Constructs the user interface by creating a vertical layout with a top horizontal layout containing various controls. 

3. **model\_in\_dataBase:** 

   Verifies whether predefined model files are already stored in the database. It maps local file paths to model names, checks if each model exists (using model\_in\_db()), and if not found, extracts details from the filename and inserts a new record via insert\_new\_model(). 

4. **list\_vtk\_files:** 

   Initiates retrieval of VTK files from a remote server. It clears the text display area and calls the recursive method get\_vtk\_files() with a base URL (e.g., https://data.darts.isas.jaxa.jp/pub/hayabusa2/tir\_bundle/data\_map/proximity/). 

5. **get\_vtk\_files:** 

   Recursively navigates a given URL (up to a specified depth) to search for VTK files. It uses BeautifulSoup to parse HTML content, extracts link elements, and determines if a link points to a directory or a VTK file. For directories, it calls itself with a reduced depth; for VTK files, it calls try\_insert\_file to download and process the file. In case of errors, it calls get\_next\_folder(). 

6. **get\_next\_folder:** 

   Parses the page for folder links and returns the URL of the first available folder, if any. 

7. **try\_insert\_file:** 

   Downloads a VTK file from a given URL using a requests session with custom headers. After downloading, it temporarily saves the file locally, reads its content to extract important details (mission name, camera timestamp, file format, model type, and size), moves the file into a structured local directory based on its model details, and records the mission information in the database. 

8. **convert\_csv\_vtk:** 

   Handles conversion of a CSV file into a VTK file enriched with cell data. It creates an output VTK filename based on the selected model and the CSV file’s basename, then calls combine\_vtk\_csv() with the current model’s VTK file path, the new CSV file path, and the derived output filename. 

9. **update\_model:** 

   Triggered when the user selects a different model from the combo box. Updates the current model information by fetching the corresponding VTK file path from the database (using get\_model()) and storing it in self.model. 

10. **open\_file\_dialog:** 

    Opens a file dialog to allow the user to select a CSV file for processing. Once a file is selected, it updates a line edit widget with the file path and constructs a new CSV filename based on the current model selection and designated output folder. The CSV file is then processed accordingly. 

### **selection.py** 

This file contains three classes: RigionType, PyVistaWidget, and Selections. 

*RigionType Class* 

- **\_\_init\_\_:** 

  Initializes the region data structure by setting up attributes to hold data extracted from VTK files, later used for CSV exports. 

- **create\_csv:** 

  Accepts a list of dictionaries and writes the collected data to a CSV file named data\_for\_analysis.csv. It defines CSV headers, iterates over data entries (converting types as needed), writes each row, and prints a confirmation message upon success. 

- **initialize\_data\_structure:** 

  Initializes and returns an empty list to serve as the container for region data entries. 

- **all\_similar\_rigions\_data:** 

  Retrieves file paths from the database matching the current model type and size, processes each file by reading the mesh, computing normals (if missing), extracting cell measurements and positional data, constructing timestamps from filenames, and aggregating data into dictionaries. The aggregated data is then passed to create\_csv to export the consolidated dataset. 

- **create\_vrk:** 

  A placeholder method for creating or handling VTK data. 

- **write\_vtk\_polydata:** 

  Writes the region’s VTK polydata to a file. It compiles stored data into VTK format, including a file header with model-specific descriptions, and writes points, polygons, and cell data sections. 

*PyVistaWidget Class* 

- **\_\_init\_\_:** 

  Creates a widget for displaying PyVista renderings within a Qt application. It sets up a vertical layout, initializes the interactor, and adds it to the layout to provide a dedicated area for 3D visualization. 

- **update\_plot:** 

  Updates the 3D plot by clearing previous content, computing the mean curvature of the provided mesh, assigning the computed curvature to the mesh’s point data, adding the mesh to the plotter with a specified colormap, and refreshing the display. 

*create\_ipython\_widget Function:* 

This standalone function sets up an embedded IPython console within the application. It creates a RichJupyterWidget and a corresponding kernel manager and client, configuring the kernel command to launch an IPython kernel. After starting the kernel and its communication channels, the function assigns the kernel manager and client to the console widget and sets a custom banner. It then executes a snippet of code to pre- import common libraries into the kernel environment. 

*Selections Class* 

- **\_\_init\_\_:** 

  Initializes the selection widget responsible for managing region selections and data export. It creates an embedded IPython console, sets up data stacks for points, polygons, and scalar values, initializes the current mesh and console flags, and tests the embedded console. 

- **initUI:** 

  Constructs the graphical user interface for the selection tool. It organizes the layout into left and right sections, including controls for date selection, model type and size, buttons for starting selections and downloading data, a 3D visualization widget, a 2D plot canvas, and an embedded IPython console. 

- **toggle\_console:** 

  Toggles the visibility of the embedded IPython console and updates the button text accordingly. 

- **refresh\_plot:** 

  A helper method to refresh the current plot display. 

- **plot\_console\_data:** 

  Creates a scatter plot on the canvas using provided x and y data, colored by scalar values. Clears previous content, plots data with a colormap, adds a colorbar with a label, sets titles and axis labels, and redraws the canvas. 

- **push\_conole:** 

  Pushes a set of variables into the embedded IPython console by iterating through a dictionary, constructing assignment commands, and sending them to the kernel client. 

- **push\_data:** 

  Wraps the process of pushing multiple variables to the console by calling push\_conole with keyword arguments, executes a code snippet to confirm the pushed variables, and sets a test variable. 

- **test\_console\_execution:** 

  Sends a test code snippet to the embedded IPython console to verify that code execution works correctly, printing a confirmation message and setting a test variable. 

- **download\_csv:** 

  Initiates the process of compiling and downloading CSV data for the selected region. 

- **all\_similar\_rigions\_data:** 

  Processes relevant VTK files and exports the aggregated data to a CSV file. 

- **new\_selection:** 

  Toggles cell selection mode. If activated, it calls start\_selection and updates the button text to indicate active selection; if deactivated, it calls stop\_selection and resets the button text. 

- **stop\_selection:** 

  Disables cell picking in the PyVista widget, clears the list of selected cells, and updates the display. 

- **start\_selection:** 

  Enables cell picking on the loaded mesh. Validates that a mesh exists and contains cells, clears the current plot, re-adds the mesh with computed curvature for context, and enables cell picking with a callback (callback\_cell\_pick). 

- **clean\_contiune\_select:** 

  Manages “continue selection” mode by either clearing all accumulated selection data or setting a flag to allow continuous accumulation, updating the button text accordingly. 

- **callback\_cell\_pick:** 

  Triggered when a cell is picked in the 3D view. Checks for valid cell IDs, updates the selection, extracts the corresponding mesh subset, and calls update\_region\_data with the filtered mesh. 

- **update\_region\_data:** 

  Aggregates and updates region data based on selected mesh cells by extracting point and polygon information, retrieving scalar data, and concatenating data from successive selections. 

- **update\_visualization:** 

  Prints the current visualization option selected in the combo box. 

- **plot:** 

  Handles creation of a 2D scatter plot on the canvas. Determines whether to plot geographical data or pixel coordinates based on user selection, retrieves corresponding scalar data for coloring, adjusts axis limits, adds a colorbar, sets up a rectangle selector for interactive selection, and pushes plotted data to the console. 

- **onselect\_rectangle:** 

  A callback executed after a rectangle selection is completed on the plot. Calculates selection bounds, identifies data points within these bounds, and highlights the selected points. 

- **update:** 

  A convenience method that calls plot to refresh the visualization. 

- **download\_vtk:** 

  Exports the current region selection data to a VTK file by calling the write\_vtk\_polydata method with a specified filename. 

- **send\_date:** 

  Extracts date and time components from the date-time edit widget and uses them— along with the currently selected model type and size—to query the database via get\_name. If a matching file is found, it loads the mesh and calls setupVisualizationType to update available scalar fields. 

- **setupVisualizationType:** 

  After loading a mesh, retrieves array names from the mesh’s first block, filters out non-relevant fields, and populates the scalar type combo box with the remaining options. 

- **updateVisualizationType:** 

  Updates the 3D visualization by clearing the current plotter and re-adding the mesh with the scalar data selected from the scalar type combo box. 

- **update\_model\_size:** 

  Triggered when the model type selection changes; updates the model size combo box with appropriate options. For example, if the model type is "sfm", sizes such as "50k", "200k", and "800k" are provided; if "spc" is selected, a different set of options is offered. 

- **set\_valid\_dates:** 

  Retrieves valid date ranges from the database by calling get\_dates and get\_dates\_day, parses these dates into Python datetime objects, and returns both complete and simplified lists of dates. 

- **check\_date:** 

  Validates the user-selected date by comparing it to allowed dates (converted into QDateTime objects). If the date is not allowed, it finds and sets the nearest valid date. 

- **load\_css:** 

  Attempts to load and apply a CSS stylesheet to the widget from a specified file path. 

### **rigions\_analysis\_functions.py:** 

This file contains three classes: MeshLoader, MeshAnalyzer, and MeshVisualizer. *MeshLoader Class:* 

- **\_\_init\_\_**:

  This constructor simply saves the provided file path and initializes an internal variable to hold the mesh later. 

- **load\_and\_align\_mesh:** 

  This method loads a mesh from a file and converts its cell data to point data. After loading, it aligns the mesh by using PCA to to determine the principal axes. A rotation matrix is constructed based on the PCA ouput with a conditinal flip og the z axis if necessary to reorient the mesh. The fucnion computes the centoid of the mesh in the XY plane by averaging the x and y coordinates over all points, and then translates every point by subtracting the centroid. Finally, the z-coordinates are extracted and stored as an additional data array in the mesh 

- **mesh:** 

  This method returns the processed mesh after it has been aligned and translated. 

- **\_print\_extreme\_points:** 

  This helper function examines the z-coordinates of the mesh points to identify the extreme values. The highest point is determined by finding the point with the maximum z-value, and the lowest point is found by locating the point with the minimum z-value. 

*MeshAnalyzer Class:* 

- **\_\_init\_\_**: 

  The constructor accepts a preprocessed mesh and stores it. 

- **compute\_geodesic\_paths** 

  This method calculates the shortest paths along the mesh surface between every unique pair of the provided points. For each pair, the closest corresponding points on the mesh are identified, and compute the geodesic path. The resulting path vertices are then collected in a dictionary, where each key corresponds to a point pair. 

- **find\_path\_intersection:** 

  This method determines the intersection between two geodesic paths by converting the lists of vertices into sets of coordinate tuples and then computing their intersection. The common points found in both sets are returned as the intersection. 

- **trace\_boundary:** 

  This method constructs a closed boundary along the mesh by connecting consecutive boundary points with geodesic paths. For each adjacent pair of boundary points a geodesic path is computed. The method then manually checks consecutive segments by comparing their endpoints using a tolerance condition. The traced points are assembled into a closed loop, and a polyline is created based on a manually constructed connectivity list. In addition to the closed boundary, the method returns the unique boundary points, the set of face IDs that constitute the boundary, and the individual path segments. 

- **fit\_circle\_to\_points**: 

  This function fits a circle to a set of 2D points withoud z axis by defining a custom cost function. For each point (*xi*,*yi*), the cost is the squared difference between the distance from the point to a proposed circle (with center (*cx*,*cy*) and radius *r*) and the radius *r* itself:  **![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.107.png)**

- **generate\_circular\_boundary:** 

  Based on the circle parameters (center and radius), this function generates a set of points that form a circular boundary. It computes evenly spaced angles between 0 and 2π and calculates the corresponding *x* and *y* coordinates using:** 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.108.png)

for *i*=1,…,*num\_points*. Custom interpolation logic is then applied to assign a z- coordinate to each (*xi*,*yi*) coordinate, resulting in a set of 3D points that describe the circular boundary.** 

- **find\_closest\_points\_on\_mesh:** 

  This method identifies, for each input point, the closest point on the mesh by calculating the Euclidean distance:** 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.109.png)

for each mesh point qqq and selecting the one with the minimum distance. The process is implemented as a loop over the input points.** 

- **compute\_centroid\_of\_points:** 

  This method computes the centroid (geometric center) of a collection of points by averaging their coordinates: ![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.110.png)

  This arithmetic mean provides a simple measure of the center of the points. 

- **find\_vertical\_intersection:** 

  This function simulates a vertical ray cast from a given start point in the negative z- direction. The ray is defined by adding a large multiple of the vector [0,0,−1] to the start point: **![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.111.png)**

  A ray-tracing routine then determines the first intersection between this ray and the mesh surface, providing a reference point for vertical measurements.** 

- **iterate\_to\_boundary:** 

  This method performs a breadth-first search starting from the cell closest to the start point to find all cells that are considered inside a boundary. For each cell, the distance between its vertices and any of the given boundary points is checked. If the distance satisfies ∥*v*−*b*∥< *ϵ* (epsilon *ϵ* roughly 10**-3**), the cell is marked as a boundary cell; otherwise, it is considered to be inside. The set of all visited cells, once the boundary is reached, defines the interior region.** 

- **compute\_centroid\_of\_cells:** 

  This function computes the centroid of a collection of cells by aggregating all the vertices from these cells and calculating their arithmetic mean:  

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.112.png)

This provides a central location for the selected region.** 

- **compute\_crater\_slope:** 

  For each cell within the selected region, the function first obtains the cell’s normal vector and then manually normalizes it. The angle between the normalized normal and the vertical direction [0,0,1] is computed using the arccosine of their dot product. The result is converted from radians to degrees. Depending on the chosen method, the function returns either the average or the maximum of these angles as a measure of the crater’s slope (now only is using avarage).** 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.113.png)

- **compute\_geodesic\_diameter**: 

  Although geodesic paths are computed using a library routine, this function custom- aggregates the results by iterating over every unique pair of boundary points, measuring the length of each geodesic path, and defining the geodesic diameter *D* as the maximum length observed: **![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.114.png)**

This provides a measure of the largest surface distance within the boundary.** *MeshVisualizer Class:* 

- **\_\_init\_\_**: 

  The constructor connects the mesh loader and analyzer components and initializes an empty list for storing points picked interactively.  

- **start\_interactive\_plot:** 

  This method sets up the mesh for interactive point picking by ensuring that the mesh is loaded and by extracting extreme z-values.. It establishes a callback mechanism to record the points as they are picked. 

- **\_on\_point\_picked:** 

  Each time a point is picked interactively, this callback adds it to an list. Once the number of picked points reaches a predefined maximum. 

- **\_run\_analysis\_and\_visualization**: 

  After collecting the required number of points, this function coordinates several custom analyses. It computes geodesic paths among the picked points, identifies specific point pairs and searches for intersections between their paths. It then calls the trace\_boundary function to form a closed boundary around the picked points and uses the custom circle-fitting function to determine a circle that best fits these boundary points. A circular boundary is generated, and the closest mesh points to this boundary are identified. Cells within the boundary are then determined, and the geodesic diameter is computed from these cells. Finally, the function visualize depth and angle measurements based on the analysis. 

- **run\_analysis\_without\_plotting:** 

  This method computes geodesic paths, determines intersections, traces the boundary, fits a circle to obtain crater dimensions computes the centroid of the inside region, and calculates vertical depth and the inclination angle: 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.115.png)

- **compute\_depth\_angle:** 

  This function calculates the vertical depth and the angle of inclination within the selected region. It first computes the centroid of the inside faces, then finds a vertical intersection point by casting a downward ray. The highest point in the region is identified, and the depth is determined as the difference in their z-values: 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.116.png)

The horizontal distance is computed as: 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.117.png)

and the inclination anfle is detemined using:** 

![](Aspose.Words.ac6c05da-6eb6-49cb-b505-a418b3230bfc.118.png)

the results are converted to degrees.** 

- **plot\_depth\_angle\_between\_A\_B:** 

  This method. extracts the cells of the region, identifies the vertical intersection and the highest point, and computes both the depth difference and the angle of inclination as described previously. In addition, it iterates over each cell in the region to calculate individual slopes and stores these slopes in the mesh’s cell data.** 

### **rigions.py:** 

This file contains one class: Regions. *Regions Class:* 

- **\_\_init\_\_**: 

  Initializes the Regions widget by constructing a split layout. It sets up some widgets.   

- **load\_and\_align\_mesh:** 

  It is a file dialog to select a VTK file, then instantiates and aligns the mesh using **MeshLoader**. It also creates **MeshAnalyzer** and **MeshVisualizer** objects for subsequent analysis and visualization. The loaded mesh is displayed in the embedded PyVista plotter, colored according to a "Z-Gradient" scalar field. 

- **enable\_picking:** 

  Prepares the 3D scene for point picking by disabling any existing picking actions, clearing previously picked points and their visual markers, and resetting the point counter label. It then enables point picking in the plotter, specifying a callback point\_picked to record each selected point. 

- **disable\_current\_picking:** 

  Attempts to disable any ongoing point-picking operation in the plotter. It clears the list of picked points and updates the corresponding label.** 

- **point\_picked:** 

  Executed each time a point is picked in the 3D view. It adds the new point’s coordinates to the list of picked points, places a small red sphere at the pick location for visualisation, and updates the label that shows how many points have been selected. Once the target number of points is reached, it enables the analysis and reset buttons.** 

- **run\_analysis:** 

  Checks if the correct number of points has been picked and, if so, calls the visualizer’s run\_analysis\_without\_plotting method to process these points. The analysis calculates depth, angle, crater slope, geodesic measurements, and returns these metrics in a dictionary. This function updates the result labels with the computed values.** 

- **visualize\_angle:** 

  Removes the small red dots indicating picked points and validates that the analysis results are available. It extracts the "inside faces" from the analysis results and uses them to highlight the selected region. It then computes a vertical intersection point and the highest point in the region, calculating the depth and angle of inclination using an arctan approach. For visual clarity, it draws a line between these two points and an arc that represents the transition from the horizontal plane to the computed vector. A custom rotate\_vector function is used to incrementally rotate a horizontal vector about a computed axis, generating a smooth arc for the visualizaiton.** 

- **reset\_selection**: 

  Resets the picking state and analysis results to allow a new selection process. It clears previously picked points, empties the result labels, and restores the mesh to its initial display. The user can then enable picking again and start a fresh analysis.** 

- **add\_region\_db:** 

  Prompts the user for a region name and, if provided, checks whether analysis results are available. If they are, it extracts the computed metrics  and calls a helper method insert\_region\_into\_db to store these in the database.*  

- **insert\_region\_into\_db:** 

  Receives the region name and computed crater metrics, converts them to floats if necessary, and inserted in to the database using dbInsert. This allows the region’s data to be stored persistently.***  

- **load\_css:** 

  Attempts to load and apply a CSS stylesheet to the widget from a specified file path. 

### **analysis\_page.py:** 

This file contains one class: AnalysisPage. *AnalysisPage Class:* 

- **\_\_init\_\_**: 

  Constructs the AnalysisPage window by setting different pages: **Selections** and **Regions** and adds them to the central widget.  

**Time\_page.py** 

This file contains three classes: PyVistaWidget, and TimePage. *PyVistaWidget Class* 

- **\_\_init\_\_:** 

  Initializes the TimePage widget by calling initUI(), setting up internal variables and preparing the widget for later use. 

- **initUI:** 

  This method sets up the overall user interface using a vertical layout. A top section contains a combo box for selecting the number of views. Below that, a grid layout is created to hold individual view containers. Each view will later include controls for date selection, model options, visualization settings, and an embedded PyVista plot. 

- **selectionchange:** 

  Triggered when the number of views is changed via the combo box. It converts the selected text to an integer and calls update\_views() with that number if it is greater than zero, ensuring that the layout reflects the user’s desired view count. 

- **get\_grid\_dimensions:** 

  Calculates and returns the grid dimensions for displaying a given number of views. For instance, if there are up to 3 views, it returns a single row; for 4–6 views, it returns two rows; and for 7–9 views, it returns a 3×3 grid. This helps in dynamically arranging view containers. 

- **update\_views:** 

  Clears any existing content from the grid layout and resets the list of views. It then determines the appropriate number of rows and columns using get\_grid\_dimensions(), sets stretch factors to make the grid expandable, and creates a new container widget for each view. Each container is built with a header, a middle section, and a bottom section. These controls are stored in a dictionary for each view and added to the grid layout.** 

- **load\_meshes:** 

  For a specified view, this function retrieves the selected date range and model options from the corresponding controls, then queries the database for file names matching those parameters. It caches the meshes and sets the slider’s maximum value based on the number of cached files. The initial mesh is loaded from the cache, visualization options are updated via setupVisualizationType(), and the mesh is added to the plotter. This function orchestrates the loading and caching of mesh data for a view.** 

- **one\_load:** 

  Updates the current mesh index for a view and retrieves the corresponding mesh from the cache. It then updates the plot by calling the update\_plot() method of the view’s PyVista widget with the selected scalar visualization. This function handles switching the displayed mesh based on slider changes.** 

- **setupVisualizationType:** 

  For a given view, this method retrieves the current mesh from the cache and extracts its scalar array names. It filters out certain arrays and populates the visualization combo box with the remaining options. If a previous mesh is present, it is removed before updating.**  

- **updateVisualizationType:** 

  This function triggered when the user selects a different visualization type. It retrieves the current scalar name from the combo box and the corresponding mesh from the cache, then calls the view’s update\_plot() method to refresh the display with the new scalar data. This function dynamically changes the visual representation of the mesh.** 

- **toggle\_animation:** 

  This function toggles the animation state for a specified view. If animation is active, it stops the view’s QTimer and changes the button. This function manages the state of automatic mesh switching.** 

- **animate\_mesh:** 

  Called on timer timeout, this function updates the visualization type for the view and increments the slider value. 

- **slider\_changed:** 

  This method handles changes to the slider’s value. If the slider’s value differs from the current mesh index, it calls one\_load() to update the mesh display. It also extracts a timestamp from the file name  and updates a label to display the current time.**  

- **set\_valid\_dates:** 

  This function queries the database to retrieve valid complete and simplified date strings using two separate functions, then parses these strings into datetime objects.  

- **check\_date:** 

  When a date edit’s value changes, this function verifies whether the selected date is among the allowed dates. If the selected date is not allowed, it finds the nearest allowed date and updates the date edit accordingly.** 

- **update\_model\_size:** 

  When the model selection changes for a given view, this function clears and repopulates the model size combo box with the appropriate options. 

- **load\_css:** 

  Load and apply a CSS stylesheet to the widget from a specified file path. 

**multiview\_page.py** 

This file contains two classes: PyVistaWidget, and MultiviewPage. *PyVistaWidget Class* 

- **\_\_init\_\_:** 

  This constructor initializes the widget by creating a vertical layout and embedding a plottwe. The plotter is configured with a default view vector and added to the layout, with the widget set to expand fully. 

- **update\_plot:** 

  This function clears the current plot and then adds the provided mesh to the plotter using the specified scalar data for coloring. If the scalar is available in the mesh, it is displayed along with its scalar bar. 

*MultiviewPage Class* 

- **\_\_init\_\_:** 

  This cunstructor initializes the MultiviewPage widget by setting the window title and preparing an empty list for view references. It then calls initUI() to build the user interface. This class serves as a container for multiple 3D views, each with its own controls for time selection, model options, and visualization settings. 

- **initUI:** 

  This funciton Constructs the overall layout for the page. The function also applies a CSS stylesheet for styling. 

- **load\_css:** 

  Load and apply a CSS stylesheet to the widget from a specified file path.** 

- **selectionchange:** 

  Triggered when the combo box selection changes. If the user selects a valid number of views, it converts the selection to an integer and calls update\_views() to create that number of view containers in the grid layout.** 

- **get\_grid\_dimensions:** 

  This mehtod determines the appropriate grid dimensions based on the total number of views. For example, up to 3 views are arranged in a single row; 4–6 views use two rows; and 7–9 views are arranged in a 3×3 grid** 

- **update\_views:** 

  This method clears any existing view widgets from the grid layout and resets the views list. Using the grid dimensions from get\_grid\_dimensions(), it sets stretch factors for rows and columns to ensure the views expand evenly. For each view slot, it creates a container widget that includes a header and a PyVistaWidget for 3D rendering. It connects the appropriate signals to their handler functions.** 

- **update\_model\_size:** 

  Updates the model size combo box based on the currently selected model from the model combo box** 

- **check\_date:** 

  This compares the selected date against a list of allowed dates. If the selected 

  date is not valid, it finds the nearest allowed date and resets the date/time edit accordingly, ensuring only valid date selections are used.

- **set\_valid\_dates:** 

  This method queries the database for valid dates by calling two functions: one returns complete date-time strings and the other returns simplified date strings. It parses these strings into datetime objects.** 

- **handle\_datetime\_change:** 

  Triggered when the “Set Date” button is pressed for a view. It extracts the date and time components from the corresponding time edit, retrieves the selected model and model size, and calls a database function to obtain the file name for that datetime and model. If a file is found, the mesh is read from that file and the view’s visualization options are updated accordingly by calling setupVisualizationType().** 

- **setupVisualizationType:** 

  After a mesh is loaded for a view, this function extracts the available scalar array names from the mesh and populates the visualization combo box with these options.**  

- **updateVisualizationType:** 

  Tis methid is triggered when the user changes the selected visualization type for a view. It retrieves the chosen scalar name from the combo box and calls the update\_plot() method of the view’s PyVistaWidget to refresh the 3D display using 

  the new scalar data.** 

**main.py** 

This file contains two classes: MainWindow. *MainWindow Class* 

- **\_\_init\_\_:** 

  This constructs the main application window by setting the title and geometry. It creates a central to manage multiple pages and instantiates four pages which are added to the stack. A bottom toolbar is then created with a horizontal layout and custom styling. The toolbar contains several widgets that switches the displayed page in the central widget.  
