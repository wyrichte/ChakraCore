import argparse
import os
import sys

from git_ch.gitch import GitCh


class GitChCommand(object):
    pass


class StatusCommand(GitChCommand):
    @classmethod
    def handle_argparse(class_name, subparser):
        parser = subparser.add_parser('status', help='Show the status of the repos')
        parser.add_argument('--summary', action='store_true', help="Prints what the head of each repo is")
        parser.add_argument('--diff', action='store_true', help="Shows the composite diff of unstaged and staged files to committed files across repos")
        parser.set_defaults(func=class_name.run)

    @staticmethod
    def run(args, unknown_args):
        gitch = GitCh()
        if (args.diff):
            gitch.diff_index()
        elif (args.summary):
            gitch.status_summary()
        else:
            gitch.status(' '.join(unknown_args))

class FetchCommand(GitChCommand):
    @classmethod
    def handle_argparse(class_name, subparser):
        parser = subparser.add_parser('fetch', help='Fetch refs from remote repos')
        parser.set_defaults(func=class_name.run)

    @staticmethod
    def run(args, unknown_args):
        GitCh().fetch(' '.join(unknown_args))

class SwitchCommand(GitChCommand):
    @classmethod
    def handle_argparse(class_name, subparser):
        parser = subparser.add_parser('switch', help='Create or switch to branch in both repos')
        parser.add_argument('branch_name', help='Branch to create or switch to')
        parser.add_argument('--buildable', action='store_true', help='Use this flag if you intend to build this branch on VSO')
        parser.add_argument('--literal', action='store_true', help='Interpret branch name literally')
        parser.set_defaults(func=class_name.run)

    @staticmethod
    def run(args, unknown_args):
        GitCh().switch(args)

    
def main():
    parser = argparse.ArgumentParser(prog='git ch')
    subparser = parser.add_subparsers(title="Subcommands", help="sub-command help")

    [cls.handle_argparse(subparser) for cls in GitChCommand.__subclasses__()]

    if (len(sys.argv) == 1):
        parser.print_help()
        return

    args, unknown_args = parser.parse_known_args()
    try:
        args.func(args, unknown_args)
    except KeyboardInterrupt:
        raise SystemExit("Operation aborted")

if __name__ == "__main__":
    main()
