import enum
import itertools
import statistics

from plotly import graph_objects, express


class Cube(enum.IntEnum):
    EMPTY = 0
    SOLID = 1
    NORMAL = 2


def oc_get_values(func):
    cube_types = [Cube.EMPTY, Cube.SOLID, Cube.NORMAL]
    values = [0, 0, 0]
    for idx in range(len(cube_types)):
        values[idx] = func(cube_types[idx])
    return values


def oc_min(form):
    values = oc_get_values(form.min)

    val = min(values)
    return 8 * val


def oc_avg(form):
    def iter_f():
        itr = itertools.combinations_with_replacement(range(3), 8)
        next(itr)  # drop empty cube
        for cubes in itertools.combinations_with_replacement(range(3), 8):
            if cubes == (2, 2, 2, 2, 2, 2, 2, 2):
                continue  # drop SOLID empty
            c_sum = 0
            for cube in cubes:
                c_sum += form.avg(cube)
            yield c_sum / 8

    return statistics.median(iter_f())


def oc_max(form):
    values = oc_get_values(form.max)

    val = max(values)
    return 8 * val


class Sauerbraten:
    @staticmethod
    def name():
        return "Sauerbraten"

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return 2 + 12 * 8

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return 2 + 12 * 8

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return 2 + 12 * 8


class Mark1:
    @staticmethod
    def name():
        return "Inexor I"

    @staticmethod
    def calc(value):
        """NORMAL cube size"""
        return 2 + 8 * 3 + value * 3

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(1)

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            def iter_f():
                for i in range(1, 3 * 8 + 1):
                    yield cls.calc(i)

            return statistics.median(iter_f())

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(8 * 3)


class Mark2:
    @staticmethod
    def name():
        return "Inexor II"

    @staticmethod
    def calc(one_sided, double_sided):
        """NORMAL cube size"""
        return 2 + 12 * 2 + one_sided * 3 + double_sided * 5

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(1, 0)

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            def iter_f():
                itr = itertools.combinations_with_replacement(range(3), 12)
                next(itr)
                for i in itr:
                    values = i
                    yield cls.calc(values.count(1), values.count(2))

            return statistics.median(iter_f())

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(0, 12)


def plotly(forms, fig, categories):
    max_list = [0] * 12
    form_list = []
    for form in forms:
        values = []
        for typ in [Cube.EMPTY, Cube.SOLID, Cube.NORMAL]:
            values.extend([form.min(typ), form.avg(typ), form.max(typ)])
        values.extend([oc_min(form), oc_avg(form), oc_max(form)])
        form_list.append(values)

    for form_v in form_list:
        max_list = list(map(max, max_list, form_v))

    for idx in range(len(form_list)):
        form_v = form_list[idx]
        form = forms[idx]
        fig.add_trace(graph_objects.Scatterpolar(
            r=list(map(lambda a, b: 100 / a * b, max_list, form_v)),
            text=form_v,
            theta=categories,
            fill='toself',
            name=form.name()
        ))


if __name__ == '__main__':
    categories = ['Empty (min)', 'Empty (avg)', 'Empty (max)',
                  'Solid (min)', 'Solid (avg)', 'Solid (max)',
                  'Normal (min)', 'Normal (avg)', 'Normal (max)',
                  'Octant (min)', 'Octant (avg)', 'Octant (max)']

    fig = graph_objects.Figure()
    plotly([Sauerbraten, Mark1, Mark2], fig, categories)

    fig.update_layout(
        title="Relative structure size comparison in bits",
        polar=dict(
            radialaxis=dict(
                visible=True,
                range=[0, 100]
            )),
        showlegend=True
    )

    fig.write_html("radar.html")
