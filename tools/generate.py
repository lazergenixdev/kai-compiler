import argparse

def expand(s: str) -> str:
    return " " + s + " "

def section(args):
    print("//".ljust(100, "="))
    print("//".ljust(20, " ") + expand(args.text))
    print("//".ljust(100, "="))

def main():
    parser = argparse.ArgumentParser(prog="generate", description="Utility to generate code/comments")
    subparsers = parser.add_subparsers(dest="command")

    p = subparsers.add_parser("section", help="Initialize a new repository.")
    p.add_argument("text")
    #p.add_argument("-m", "--message", required=True, help="Commit message.")
    p.set_defaults(func=section)

    args = parser.parse_args()

    if args.command:
        args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
