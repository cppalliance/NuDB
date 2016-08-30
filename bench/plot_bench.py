#/usr/bin/env python

# Script to read the result of the benchmark program and plot the results.
# Options:
#   `-i arg` : input file (benchmark result)
#   `-o arg` : html output for the plot
# Notes: After the script runs the plot will automatically be shown in a browser.
#        Tested with python 3 only.

import argparse
import itertools
import collections
import pandas as pd
import matplotlib.pyplot as plt
import re

from bokeh.layouts import gridplot
from bokeh.palettes import Spectral11
from bokeh.plotting import figure, show, output_file


# Given a file at the start of a test result (on the header line)
# Return a data frame for the test result and leave the file one past
# the blank line at the end of the result
def to_data_frame(header, it):
    column_labels = re.split('  +', header.strip())
    columns = [[] for i in range(len(column_labels))]
    for l in it:
        l = l.strip()
        if not l:
            break
        fields = l.split()
        if len(fields) != len(columns):
            raise Exception('Bad file format, line: {}'.format(l))
        for c, f in zip(columns, fields):
            c.append(float(f))
    d = {k: v for k, v in zip(column_labels, columns)}
    return pd.DataFrame(d, columns=column_labels[1:], index=columns[0])


def to_data_frames(f):
    trial = ''
    result = {}
    for l in f:
        if l and l[0] == '#': continue
        if l and l[0] == ' ' and l.strip():
            if trial:
                # Remove anything in parens
                trial = re.sub('\([^\)]*\)', '', trial)
                result[trial] = to_data_frame(l, f)
            trial = ''
            continue
        if trial: trial += ' '  # Handle multi-line labels
        trial += l.strip()
    return result


def bokeh_plot(title, df):
    numlines = len(df.columns)
    palette = Spectral11[0:numlines]
    p = figure(
        width=500,
        height=400,
        title=title,
        x_axis_label='DB Items',
        y_axis_label='Ops/Sec.')
    for col_idx in range(numlines):
        p.line(
            x=df.index.values,
            y=df.iloc[:, col_idx],
            legend=df.columns[col_idx],
            line_color=palette[col_idx],
            line_width=5)
    return p


def run_main(result_filename, plot_output):
    with open(result_filename) as f:
        dfd = to_data_frames(f)
        plots = []
        for k, v in dfd.items():
            plots.append(bokeh_plot(k, v))
        output_file(plot_output, title="NuDB Benchmark")
        show(gridplot(*plots, ncols=2, plot_width=500, plot_height=400))
        return dfd  # for testing


def parse_args():
    parser = argparse.ArgumentParser(
        description=('Plot the benchmark results'))
    parser.add_argument(
        '--input',
        '-i',
        help=('input'), )
    parser.add_argument(
        '--output',
        '-o',
        help=('output'), )
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    result_filename = args.input
    plot_output = args.output
    if not result_filename:
        print('No result file specified. Exiting')
    elif not plot_output:
        print('No output file specified. Exiting')
    else:
        run_main(result_filename, plot_output)
