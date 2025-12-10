import re

def agm_to_mtx(input_file, output_file):

    vertices = {}
    hyperedges = []

    with open(input_file) as f:
        lines = f.readlines()

    i = 0
    num_vertices = 0
    seen_input = False
    while i < len(lines):
        strings = lines[i][:-2].split(" ")
        if strings[0] == "input":
            seen_input = True
        if len(strings) == 1 or not seen_input:
            i += 1
            continue
        if strings[0] in {"input", "output", "wire"} or (strings[0] in {"", "\t"} and len(strings) > 1):
            variables = strings[-1].split(",")
            for v in variables:
                if v == "":
                    continue
                print(v)
                vertices[v] = num_vertices
                num_vertices += 1
        elif strings[0] not in {"//", "module"}:
            nodes = strings[1].split("(")[1][:-1].split(",")
            curr_edge = []
            for j, string in enumerate(nodes):
                curr_edge.append(vertices[string])
            hyperedges.append(curr_edge)
        i += 1

    with open(output_file, 'w') as out:
        out.write(f"{len(hyperedges)} {num_vertices}\n")

        for vertices in hyperedges:
            #print(vertices)
            vertices = sorted(set(vertices))
            out_str = ""
            for vertex in vertices:
                out_str += str(vertex) + " "
            out.write(out_str[:-1] + "\n")

for num in [1355]:
    agm_to_mtx(f"/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/circuit_graphs/c{num}.v", f"circuit_hypergraphs/c{num}.mtx.hgr")
