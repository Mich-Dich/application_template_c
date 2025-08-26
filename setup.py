import sys
import subprocess

# Check for required Python packages
required_modules = {
    'yaml': 'PyYAML'
}

for module, package in required_modules.items():
    try:
        __import__(module)
    except ImportError:
        # Check if pip is available
        try:
            subprocess.run([sys.executable, "-m", "pip", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print(f"Error: pip is not installed. Please install pip to proceed with installing {package}.")
            sys.exit(1)

        print(f"Installing required package: {package}")
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", package])
        except subprocess.CalledProcessError:
            print(f"Failed to install {package}. Please install it manually by running: pip install {package}")
            sys.exit(1)

import os
import re
import yaml
import socket
import platform
import scripts.utils as utils
import scripts.git_util as git_util
IS_CI = os.getenv("CI") == "true"

if platform.system() == "Linux":
    from scripts.linux.setup_env import env_configuration
    from scripts.linux.setup_premake import premake_configuration
    import scripts.linux.IDE_setup as IDE_setup
elif platform.system() == "Windows":
    from scripts.windows.setup_env import env_configuration
    import scripts.windows.win_utils as win_util
    win_util.enable_ansi_support()
    from scripts.windows.setup_premake import premake_configuration
    import scripts.windows.IDE_setup as IDE_setup
else:
    raise Exception("Unsupported operating system")



def apply_settings(file_path="./config/app_settings.yml", cmake_file_path="./CMakeLists.txt"):
    try:
        with open(file_path, 'r') as stream:            # Load app settings
            data = yaml.safe_load(stream)
            app_name = data['general_settings']['name']
        
        with open(cmake_file_path, 'r') as file:        # Read CMakeLists.txt content
            content = file.read()

        # Regex to replace the project name while preserving language and comments
        # Matches: project(<old_name> C) optionally with trailing comment
        content = re.sub(
            r'(^\s*project\s*\()\s*([^\s\)]+)(\s+C\s*\).*)',
            r'\1' + app_name + r'\3',
            content,
            flags=re.MULTILINE
        )

        with open(cmake_file_path, 'w') as file:        # Write the modified content back
            file.write(content)
            
    except Exception as e:
        utils.print_c(f"Error while applying settings: {str(e)}", "red")
        sys.exit(1)


def get_application_name(file_path="./config/app_settings.yml"):
    try:
        with open(file_path, 'r') as stream:
            data = yaml.safe_load(stream)
            return data['general_settings']['name']
    except Exception as e:
        utils.print_c(f"Error while reading settings file: {str(e)}", "red")
        sys.exit(1)


def main():
    # Skip internet check in CI
    if not IS_CI:
        try:
            socket.setdefaulttimeout(3)
            socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect(("8.8.8.8", 53))
        except:
            utils.print_c("\nNo Internet connection found\n", "red")
            sys.exit(1)

    try:
        utils.print_u("\nCHECKING SYSTEM DEPENDENCIES")
        if not env_configuration.validate():
            utils.print_c("Missing required packages - setup aborted", "red")
            sys.exit(1)
    
        utils.print_u("\nINITIALIZING SUBMODULES")              # Initialize submodule configuration
        if not git_util.initialize_submodules():
            utils.print_c("Submodule initialization failed - setup aborted", "red")
            sys.exit(1)
    
        utils.print_u("\nUPDATING SUBMODULES")                  # Update submodules to desired branches
        git_util.update_submodule("vendor/glfw", "master")
        # git_util.update_submodule("vendor/glm", "master")
        # git_util.update_submodule("vendor/imgui", "docking")
        # git_util.update_submodule("vendor/implot", "master")
        # git_util.update_submodule("vendor/Catch2", "devel")


        utils.print_u("\nAPPLY SETTINGS")
        utils.print_c("Settings are defined at [./config/app_settings.yml]. after changing the settings, it is recommended to reexecute the setup script", "blue")
        application_name = get_application_name()
        apply_settings()
        print(f"name: {application_name}")
        

        # setup IDE
        utils.print_u("\nSETUP IDE")

        if IS_CI:                                       # select first detected IDE in CI
            IDEs = IDE_setup.detect_IDEs()
            selected_ide = IDEs[0]
            print(f"Selected IDE: {selected_ide}")
        else:
            selected_ide = IDE_setup.prompt_ide_selection()

        # VSCode spcific setup
        if "VSCode" in selected_ide:
            if IS_CI:
                build_config = "Debug"
            else:
                build_config = IDE_setup.prompt_build_config()
            IDE_setup.setup_vscode_configs(os.getcwd(), build_config, application_name)


        # Paths
        build_dir = "build"
        os.makedirs(build_dir, exist_ok=True)

        cmake_generator = None
        if platform.system() == "Linux":  # ---- LINUX VERSION ----
            # Select appropriate generator based on IDE
            if selected_ide == "JetBrains Rider":
                cmake_generator = "CodeBlocks - Unix Makefiles"     # Rider understands CodeBlocks projects
            elif "VSCode" in selected_ide:
                cmake_generator = "Unix Makefiles"                  # Works perfectly with VSCode
            elif "Makefile" in selected_ide:
                cmake_generator = "Unix Makefiles"
            else:
                cmake_generator = "Unix Makefiles"                  # Default fallback

            cmake_cmd = ["cmake", "-S", ".", "-B", build_dir, "-G", cmake_generator]

        else:  # ---- WINDOWS VERSION ----
            cmake_generator = "Visual Studio 17 2022"               # Default to Visual Studio 2022 solution

            if selected_ide == "VSCode":
                cmake_generator = "MinGW Makefiles"                 # MinGW for VSCode builds on Windows
            elif "Visual Studio" in selected_ide:
                if "2022" in selected_ide:
                    cmake_generator = "Visual Studio 17 2022"
                elif "2019" in selected_ide:
                    cmake_generator = "Visual Studio 16 2019"
                elif "2017" in selected_ide:
                    cmake_generator = "Visual Studio 15 2017"
            elif selected_ide == "JetBrains Rider":
                cmake_generator = "CodeBlocks - MinGW Makefiles"

            cmake_cmd = ["cmake", "-S", ".", "-B", build_dir, "-G", cmake_generator]

        # Run CMake configuration
        cmake_result = subprocess.run(cmake_cmd, text=True)

        # If CMake configuration failed
        if cmake_result.returncode != 0:
            utils.print_c(f"BUILD FAILED! CMake configuration failed [{cmake_result.returncode}]", "red")
        else:
            utils.print_c("CMake configuration successful!", "green")

            build_cmd = ["cmake", "--build", build_dir]                         # Build the project
            build_result = subprocess.run(build_cmd, text=True)
            if build_result.returncode != 0:
                utils.print_c(f"BUILD FAILED! Compilation errors occurred [{build_result.returncode}]", "red")
            else:
                utils.print_c("BUILD SUCCESSFUL!", "green")


        if "VSCode" in selected_ide:
            IDE_setup.print_vscode_help()

        # print helpful hints
        if platform.system() == "Linux":                # ---- LINUX VERSION ----
            utils.print_c("\nHelpful hints", "blue")
            print("  Apply changed premake scripts:     vendor/premake/premake5 gmake2")
            print("  Cleanup generated files:           gmake clean")
            print("  Compile application:               gmake -j")
            print("  More help:                         vendor/premake/premake5 --help OR https://premake.github.io/docs/Using-Premake/")

        else:                                           # ---- WINDOWS VERSION ----
            # Print helpful hints
            utils.print_c("\nHelpful hints for Windows", "blue")
            if "Visual Studio" in selected_ide or "Rider" in selected_ide:
                print("  Open solution file:               build/*.sln")
                print("  Clean solution:                   msbuild /t:Clean")
                print("  Build solution:                   msbuild /t:Build")
            elif selected_ide == "VSCode":
                print("  Apply changed premake scripts:    vendor/premake/premake5 gmake2")
                print("  Clean solution:                   gmake clean")
                print("  Build solution:                   gmake -j")
            print("  More help:                        vendor/premake/premake5 --help")


    except KeyboardInterrupt:
        utils.print_c("\nProcess interrupted by user.", "red")


if __name__ == "__main__":
    main()
