import os
from git import Repo
from plumbum import local
import re
import tempfile
import shutil
import sys


# Helpers
def _run_system_command(command):
    print("Running '%s'" % command)
    os.system(command)


def _exec_repo_command_in_dir(repo, operation):
    print("[REPO] %s" % repo.working_dir)
    with local.cwd(repo.working_dir):
        operation(repo)


def _print_repo_summary(repo):
    print("  Branch: %s" % repo.head.ref)


def _ensure_and_switch_to_branch(repo, target_branch):
    print("[REPO] %s" % repo.working_dir)
    branches = repo.branches
    branch_head = None
    for branch in branches:
        if branch.name == target_branch:
            branch_head = branch
            break

    if branch_head is None:
        print("Branch doesn't exist in this repo- creating")
        branch_head = repo.create_head(target_branch)

    print("Switching branch to %s" % target_branch)
    repo.head.reference = branch_head
    print("")


def _get_blob_for_path(root_tree, path):
    path_stack = []
    (parent, leaf) = os.path.split(path)
    while parent != '':
        path_stack.append(leaf)
        (parent, leaf) = os.path.split(parent)
    path_stack.append(leaf)

    current = root_tree
    while path_stack:
        item = path_stack.pop()
        try:
            current = current.join(item)
        except KeyError:
            print("%s not in git" % path)
            return None

    return current


def _get_changed_files_for_repo(repo, prefix=''):
    root_tree = repo.head.commit.tree
    proc = repo.git.status(porcelain=True, untracked_files=True, as_process=True)
    changed_files = []
    for line in proc.stdout:
        path = prefix + line.decode()[3:].rstrip('\n')
        blob = _get_blob_for_path(root_tree, path)
        changed_files.append((path, blob, repo))
    return changed_files


def _store_blob_to_file(repo, blob, target_path):    
    (parent, leaf) = os.path.split(target_path)
    print("Creating file %s in %s\n" % (leaf, parent))
    if not os.path.exists(parent):
        os.makedirs(parent)
    with open(target_path, "wb") as f:
        f.write(blob.data_stream.read())


def _copy_repo_file(repo, path, target_root):
    path = os.path.join(*path.split("/"))
    source_path = os.path.join(repo.working_dir, path)
    target_path = os.path.join(target_root, path)
    (target_parent, leaf) = os.path.split(target_path)
    if not os.path.exists(target_parent):
        os.makedirs(target_parent)
    shutil.copyfile(source_path, target_path)


def _construct_diff(filelist):
    temp_dir = tempfile.mkdtemp()
    before = os.path.join(temp_dir, "before")
    after = os.path.join(temp_dir, "after")

    print("Creating %s" % before)
    for (path, blob, repo) in filelist:
        if blob:
            _store_blob_to_file(repo, blob, os.path.join(before, *(path.split("/"))))
        _copy_repo_file(repo, path, after)

    return (temp_dir, before, after)


class GitCh():
    def __init__(self):
        REPO_ROOT = os.getenv('REPO_ROOT', os.getcwd())
        self.full_repo = Repo.init(REPO_ROOT)
        self.core_repo = Repo.init(os.path.join(REPO_ROOT, 'core'))

    def _for_each_repo(self, operation):
        _exec_repo_command_in_dir(self.full_repo, operation)
        print("")
        _exec_repo_command_in_dir(self.core_repo, operation)

    def status_summary(self):
        self._for_each_repo(_print_repo_summary)

    def diff_index(self):
        diff_tool = os.environ.get("CH_DIFFTOOL")
        if not diff_tool:
            sys.exit("Error: Please set CH_DIFFTOOL to your preferred diff tool")

        # Get files changed
        # Get files staged for commit
        # Get blobs corresponding to committed versions of these filese
        # Create directory structure
        # Diff
        changed_files = _get_changed_files_for_repo(self.full_repo) + \
                        _get_changed_files_for_repo(self.core_repo, 'core//')
        (temp_dir, before, after) = _construct_diff(changed_files)
        _run_system_command("%s %s %s" % (diff_tool, before, after))

    def status(self, arguments):
        self._for_each_repo(lambda repo:
                            _run_system_command("git status %s" % arguments))

    def fetch(self, arguments):
        self._for_each_repo(lambda repo:
                            _run_system_command("git fetch %s" % arguments))

    def switch(self, arguments):
        branch_name = arguments.branch_name
        if not arguments.literal and branch_name != 'master':
            username = local.env["USERNAME"].lower()
            expected_branch_pattern = "^(build|users)/%s/.*"
            r = re.compile(expected_branch_pattern)
            if (r.match(expected_branch_pattern) is None):
                prefix = "users"
                if (arguments.buildable):
                    prefix = "build"
                branch_name = "%s/%s/%s" % (prefix, username, branch_name)

        print("Branch to switch to: %s" % branch_name)
        _ensure_and_switch_to_branch(self.full_repo, branch_name)
        _ensure_and_switch_to_branch(self.core_repo, branch_name)
