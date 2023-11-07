import os
import argparse
import fileinput
from git import Repo, RemoteProgress
from tqdm import tqdm

embr_repo_url = "https://github.com/bid-p/embr.git"

init_files = ["project.xml", "Sconstruct"]

class GitProgressPrinter(RemoteProgress):
    def __init__(self):
        super().__init__()
        self.pbar = tqdm()

    def update(self, op_code, cur_count, max_count=None, message=""):
        self.pbar.total = max_count
        self.pbar.n = cur_count
        self.pbar.refresh()

def main():
    # Get project name from CLI argument
    parser = argparse.ArgumentParser("Initialize a new Embr project.")
    parser.add_argument("project_name", help="Name of the project to initialize.")
    args = parser.parse_args()

    # Update embr repo from develop branch
    embr_repo = Repo("embr")
    assert not embr_repo.bare
    embr_repo.git.checkout('develop')
    origin = embr_repo.remotes.origin

    # if there are any uncommitted changes, stash them
    embr_dirty = False
    if embr_repo.is_dirty():
        print("Stashing changes to embr.")
        embr_repo.git.stash("save")
        embr_dirty = True

    # Pull from origin/develop
    print("Updating Embr from origin/develop.")
    origin.pull()

    # Update submodules and print progress
    for submodule in embr_repo.submodules:
        print("Updating submodule: ", submodule)
        submodule.update(init=True, recursive=True)
    
    if embr_dirty:
        # Ask for user input to apply stashed changes
        print("Stashed changes:")
        print(embr_repo.git.stash("list"))
        apply_stash = input("Apply stashed changes? (y/n): ")
        if apply_stash == "y":
            print("Reapplying stashed changes to embr.")
            embr_repo.git.stash("apply")
        else:
            print("Discarding previous changes to embr.")
            # drop latest stash
        embr_repo.git.stash("drop")

    # Create a folder named project-name-src if it doesn't already exist
    project_name = args.project_name
    os.system(f"mkdir -p {project_name}-src")

    # Print overwrite warning and ask for y/n input
    for file in init_files:
        if os.path.exists(f"{project_name}-src/{file}"):
            print(f"\nWarning: {project_name}-src/{file} already exists.")
            overwrite = input("Overwrite? (y/n): ")
            if overwrite.lower() != "y":
                print("Skipping.")
                continue
        
        # Copy file to project-name-src/
        os.system(f"cp embr/project-init-templates/{file} {project_name}-src/")
        
        # Replace project name in project.xml and Sconstruct with project-name
        for line in fileinput.input(f"{project_name}-src/{file}", inplace=True):
            print(line.replace("project-name", project_name), end="")
    
    # Print a message to the user
    print(f"\nInitialized project {project_name}.")

if __name__ == "__main__":
    main()