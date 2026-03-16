
# Ray-Marcher
- An implementation of Ray marching on a custom rasterizer and dx11 framework previously created by me, Remora engine.
- Unlike traditional rasterization process that uses vertices for rendering, this method  uses **Sphere Tracing** to calculate mathematical surfaces defined by **Signed Distance Functions (SDF)**.
- This allows for infinite detail, seamless boolean operations such as union, subtraction, intersection, and creation of complex fractals and shapes.
- The core of the marcher is **Distance-Aided Ray Marching/Sphere Tracing** algorithm. For every pixel, we shoot a ray from camera position $\vec{r}(t) = \vec{o} + t\vec{d}$ and "march" through the 3d-space giving us ability to render images from math equations.
- ### Dependencies used :
	- [Remora Engine](https://github.com/staintheone/Remora-Engine)  is used as a framework that handles Window Creation, HLSL Shader Compilation, DirectXMath libraries for handling math along with ImGui for UI. 

### Build Instructions:
#### Prerequisites : - 
-  **Visual Studio 2022** (Recommended) or 2019.
-   **Windows 10/11 SDK** (Installed via Visual Studio Installer).
-   **C++17** or higher.

####  Step-by-Step Setup

1.  **Clone the Repository:**
    
    Bash
    ```
    git clone https://github.com/staintheone/Ray-Marching-Engine
    ```
2.  **Open the Project:**
    
    -   Open the `.sln` file in Visual Studio.
        
3.  **Configure the Environment:**
    
    -   Set the solution configuration to **Release** (for best performance) or **Debug**.
        
    -   Select platform, **x64**.
        
4.  **Include Directories:**
    
    -   Ensure the project recognizes the `includes/` folder.
        
    -   Right-click **Project -> Properties -> C/C++ -> General -> Additional Include Directories** and ensure it locates to dependencies folder.
        
