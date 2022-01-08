import sys
import getopt
import os

from typing import List, Optional


def render_template(template_file: str, output_file:str, **kwargs):
    with open(template_file, 'rt') as fp:
        template_content = fp.read()

    dirname = os.path.dirname(output_file)
    if not os.path.isdir(dirname):
        os.makedirs(dirname)

    with open(output_file, 'wt') as fp:
        fp.write(eval(f'f"""{template_content}"""', globals(), kwargs))


def main(argv: Optional[List[str]] = None):
    opts, args = getopt.getopt(argv[1:], "a:l:t:n:r:c:")
    opts = dict((i.lstrip('-'), j) for i, j in opts)

    table_name = opts.get('n')
    template_name = opts.get('t')
    table_len = int(opts.get('l', 8))
    stack_len = int(opts.get('s', 32))
    reg_args = opts.get('r', '')
    class_name = opts.get('c', table_name.title().replace('_', ''))
    render_template(template_name, args[0], table_name=table_name, table_len=table_len, class_name=class_name, stack_len=stack_len,
                    push_args=f'{os.linesep}    '.join(f'push    {i}' for i in reg_args.split(',')))


if __name__ == '__main__':
    main(sys.argv)
