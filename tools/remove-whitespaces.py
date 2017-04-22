#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  2014, By konstin (http://github/konstin)
#  2015, Modified by leyyin
#
#  Removes all trailing whitespaces and replaces all tabs with four spaces, the
#  files with a given extension in a recursively searched directory.
#  It can also count the number of code lines excluding comments and blank
#  lines.
#
#  Tested with python 2.7 and python 3

import os
import argparse
import sys


def main(directory, is_statistics, is_dry_run, extensions, comments_start):
    lines_total = 0
    lines_comments = 0
    file_counter = 0
    files_affected = 0

    for dir_path, _, file_names in os.walk(directory):
        if is_statistics:
            file_counter += len(file_names)

        for file_name in file_names:
            _, file_extension = os.path.splitext(file_name)

            # File does not have an extension
            if not file_extension:
                continue

            # Not a valid extension. Note: extensions have in the 0 position a dot, eg: '.hpp', '.cpp'
            if file_extension[1:] not in extensions:
                continue

            if is_statistics:
                files_affected += 1

            # Read whole file
            path_file = os.path.join(dir_path, file_name)
            with open(path_file, 'r') as f:
                lines = f.readlines()

            if is_statistics:
                lines_total += len(lines)

            # Scan lines
            is_modified = False
            for i, line in enumerate(lines):
                original_line = line

                # Replace tabs with four spaces
                line = line.replace('\t', '    ')

                line_rstrip = line.rstrip()
                if line_rstrip:  # Don't de-indent empty lines
                    line = line_rstrip + '\n'

                    # Count the number of comments
                    if is_statistics:
                        line_lstrip = line.lstrip()
                        if any([line_lstrip.startswith(c) for c in comments_start]):
                            lines_comments += 1

                # Indicate that we want to write to the current file
                if original_line != line:
                    lines[i] = line  # Replace original line
                    if not is_modified:
                        is_modified = True

            # Write back modified lines
            if not is_dry_run and is_modified:
                with open(path_file, 'w') as f:
                    f.writelines(lines)

    if is_statistics:
        print('Total number of files in {0}: {1}'.format(directory, file_counter))
        print('Total number of files affected in {0}: {1}'.format(directory, files_affected))
        print('Lines in total: {0}'.format(lines_total))
        print('      empty/comments: {0}'.format(lines_comments))
        print('↳ excluding comments and blank lines: {0}'.format(lines_total - lines_comments), end='\n' * 2)

    print('Finished.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Remove whitespace from C/C++ files.')
    parser.add_argument('directory', default='../src', nargs='?',
                        help='the directory where all the source files are located. (default: %(default)s)')
    parser.add_argument('--dry-run', dest='dry_run', action='store_true',
                        help='do a dry run. Do not modify/write any files. (default: %(default)s)')
    parser.add_argument('--statistics', dest='statistics', action='store_true',
                        help='display statistics (count files and lines if enabled). On by default.')
    parser.add_argument('--no-statistics', dest='statistics', action='store_false', help='do not display statistics.')
    parser.add_argument('--extensions', default=["cpp", "hpp", "c", "h"], nargs='+',
                        help='set file extensions. Eg: --extensions cpp hpp (default: %(default)s).')
    parser.add_argument('--comments-start', default=['//', '/*', '*'], nargs='+',
                        help='set how line comments start. Eg: --comments-start // \'*\'. (default: %(default)s).')
    parser.set_defaults(statistics=True)
    parser.set_defaults(dry_run=False)
    args = parser.parse_args()

    if not os.path.exists(args.directory):
        print('ERROR: The directory {0} does not exist'.format(args.directory))
        sys.exit(1)

    print(args)
    main(args.directory, args.statistics, args.dry_run, args.extensions, args.comments_start)
