import os
import argparse
import fileinput

# Write a script that copies the project.xml and Sconstruct file from project-init-templates into the current directory and replaces the project name with the name of the current directory.
def main():
    # Get project name from CLI argument
    parser = argparse.ArgumentParser("Initialize a new Embr project.")
    parser.add_argument("project_name", help="Name of the project to initialize.")
    args = parser.parse_args()

    # Print current directory name
    # print(f"Current directory name: {os.getcwd()}")

    # Create a folder named project-name-src if it doesn't already exist
    project_name = args.project_name
    os.system(f"mkdir -p {project_name}-src")

    # Copy project.xml and Sconstruct to project-name-src/
    os.system(f"cp embr/project-init-templates/project.xml {project_name}-src/")
    os.system(f"cp embr/project-init-templates/Sconstruct {project_name}-src/")

    # Replace project name in project.xml and Sconstruct with project-name
    for line in fileinput.input(f"{project_name}-src/project.xml", inplace=True):
        print(line.replace("project_name", project_name), end="")

    for line in fileinput.input(f"{project_name}-src/Sconstruct", inplace=True):
        print(line.replace("project_name", project_name), end="")
    
    # Print a message to the user
    print(f"Initialized project {project_name}.")

if __name__ == "__main__":
    main()