#!/usr/bin/env python3

import os
import re
import sys


def unique(seq):
    seen = set()
    seen_add = seen.add
    return [x for x in seq if not (x in seen or seen_add(x))]


def read_headers(root_dir):
    header_files = {}
    include_dir = os.path.join(root_dir, 'include')
    for root, dirs, files in os.walk(include_dir):
        for filename in filter(lambda f: f.endswith('.hpp'), files):
            file_path = os.path.join(root, filename)
            relative_path = file_path[len(include_dir)+1:]
            with open(file_path, 'r') as f:
                header_files[relative_path] = f.read()

    return header_files


def process_includes(include_file, header_files, included_files, already_included=None):
    result = []
    already_included = already_included if already_included is not None else set()

    for included_file in included_files.get(include_file, []):
        if included_file in already_included:
            continue

        already_included.add(included_file)
        result.extend(process_includes(included_file, header_files, included_files, already_included))

    result.append(header_files[include_file])
    return result


def generate_single_header(header_files):
    NAMESPACE_BEGIN_RE = re.compile(r'^\s*namespace\s*ulocal\s*{\s*$', re.MULTILINE)
    NAMESPACE_END_RE = re.compile(r'^\s*}\s*//\s*namespace\s*ulocal\s*$', re.MULTILINE)
    PRAGMA_RE = re.compile(r'^#pragma\s*once\s*$', re.MULTILINE)
    for header_file, header_content in header_files.items():
        new_header_content = NAMESPACE_BEGIN_RE.sub('', header_content)
        new_header_content = NAMESPACE_END_RE.sub('', new_header_content)
        new_header_content = PRAGMA_RE.sub('', new_header_content)
        header_files[header_file] = new_header_content

    INCLUDE_RE = re.compile(r'^#include\s*<(.*)>\s*$', re.MULTILINE)
    included_files = {}
    ulocal_included_files = {}
    for header_file, header_content in header_files.items():
        for match in INCLUDE_RE.findall(header_content):
            if match.startswith('ulocal/'):
                ulocal_included_files.setdefault(header_file, []).append(match)
            else:
                included_files.setdefault(header_file, []).append(match)
        header_files[header_file] = INCLUDE_RE.sub('', header_content)

    includes = []
    for i in included_files.values():
        includes.extend(i)
    includes = unique(includes)

    content = process_includes('ulocal/ulocal.hpp', header_files, ulocal_included_files)

    return r'''#pragma once

{}

namespace ulocal {{

{}

}} // namespace ulocal
'''.format(
    '\n'.join(['#include <{}>'.format(i) for i in includes]),
    ''.join(content)
)



def main():
    if len(sys.argv) != 2:
        print('Usage: {} <ROOT_ULOCAL_DIR>'.format(sys.argv[0]))
        sys.exit(1)

    root_dir = os.path.realpath(sys.argv[1])
    header_files = read_headers(root_dir)
    single_header = generate_single_header(header_files)
    print(single_header)


if __name__ == '__main__':
    main()
