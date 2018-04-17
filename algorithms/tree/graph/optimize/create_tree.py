# Used for getting input data from a file or a list
def parseNode(node):
    # node = {"ID": "C" + '{:03}'.format(rackID) + "-" + '{:02}'.format(halfrackNum) + str(nodeval), "IMG": {}};
    # node["IMG"]["data"] = {}
    # node["IMG"]["depth"] = imageDepth
    # Nodes.append(node)
    # input.append(['{:03}'.format(rackID), '{:02}'.format(halfrackNum) + str(nodeval), str(imageDepth)])
    if isinstance(node[0], int):
        return 'C' + '{:03}'.format(node[0]) + '-' + '{:03}'.format(node[1])
    else:
        return 'C' + node[0] + '-' + node[1]
    # return "C" + '{:03}'.format(rackID) + "-" + '{:02}'.format(halfrackNum) + str(nodeval)


def pre_processing2(joblist=None, inputfile=None):
    import csv
    nodes = []

    NodeMap = {}
    Groups = {}
    RankMap = {}
    if inputfile is not None:
        with open('input.csv') as f:
            f_csv = csv.reader(f)
            input = [row for row in f_csv]
    elif joblist is not None:
        input = []
        total = int(len(joblist) / 3)
        for i in range(total):
            input.append([joblist[3 * i] + joblist[3 * i + 1] + joblist[3 * i + 2]])
    else:
        return [NodeMap, Groups, RankMap]
    for index, rank in enumerate(input):
        NID = parseNode(rank)
        NodeMap[rank[2]] = NID
        RankMap[NID] = str(index)
        group = NID[0:4] + str(int(int(NID[5:7]) > 6))
        if group not in Groups:
            Groups[group] = 0
        else:
            Groups[group] = Groups[group] + 1

    return [NodeMap, Groups, RankMap]


# Used to create toy data

image_num = 64


def addNode(Nodes, nodeval=None, imageDepth=None, halfrackNum=None, rackID=None):  # , input=None):
    node = {"ID": "C" + '{:03}'.format(rackID) + "-" + '{:02}'.format(halfrackNum) + str(nodeval), "IMG": {}};
    node["IMG"]["data"] = {}
    node["IMG"]["depth"] = imageDepth
    Nodes.append(node)
    input.append(['{:03}'.format(rackID), '{:02}'.format(halfrackNum) + str(nodeval), str(imageDepth)])
    return node["ID"]


def pre_processing(image_num):
    import random
    import math
    depths = list(range(image_num))
    nodes = []
    random.shuffle(depths)
    NodeMap = {}
    Groups = {}
    RankMap = {}
    d = 0
    # Input = []
    rack_num = math.ceil(image_num / 56)
    for r in range(rack_num):
        cur_rack = random.randint(0, 999)
        for i in range(14):
            for j in range(4):
                if d >= image_num:
                    # import csv
                    # tlist = list(map(list, zip(*Input)))
                    # with open('input.csv', "w", newline='') as f:
                    #    writer = csv.writer(f)
                    #    writer.writerows(Input)

                    return [NodeMap, Groups, RankMap]
                else:
                    NID = addNode(nodes, nodeval=j, imageDepth=depths[d], halfrackNum=i,
                                  rackID=cur_rack)  # , input=Input)
                    NodeMap[str(depths[d])] = NID
                    RankMap[NID] = d
                    group = NID[0:4] + str(int(int(NID[5:7]) > 6))
                    if group not in Groups:
                        Groups[group] = 0
                    else:
                        Groups[group] = Groups[group] + 1
                    d = d + 1

    # for i in range(2):
    #    for j in range(4):
    #        NID = addNode(nodes, nodeval=j, imageDepth=depths[d], halfrackNum=i, rackID=122)
    #        NodeMap[str(depths[d])] = NID
    #        group = NID[0:4] + str(int(int(NID[5:7]) > 6))
    #        if group not in Groups:
    #            Groups[group] = 0
    #        else:
    #            Groups[group] = Groups[group]
    #        d = d + 1


# General use

# Check whether a node is in base_rack
def check_rack(NID, base_rack):
    group = NID[0:4] + str(int(int(NID[5:7]) > 6))
    if group == base_rack:
        return True
    else:
        return False


# Find base half rack to send data to
def find_base(Groups):
    import operator
    base_rack = max(Groups.items(), key=operator.itemgetter(1))[0]
    return base_rack


# Calculate Merge Order
def merge(NodeMap, base_rack):
    mergelist = [[], [], []]
    mykeys = list(NodeMap.keys())  # list(range(len(NodeMap)))
    new_ind = len(NodeMap)
    while len(mykeys) > 1:
        mykeys = list(NodeMap.keys())
        mykeys.sort(key=float)
        # print(mykeys)
        cur_key = [True] * len(mykeys)
        for index, key in enumerate(mykeys):
            if index < len(mykeys) - 1:
                if str(key) in NodeMap and str(mykeys[index + 1]) in NodeMap:
                    NID = NodeMap[str(key)]
                    NID2 = NodeMap[str(mykeys[index + 1])]
                    if not check_rack(NID, base_rack) and check_rack(NID2, base_rack) and cur_key[index] is True and \
                            cur_key[index + 1] is True:
                        mergelist[0].extend([NID, NID2])
                        mergelist[1].extend([NID2, NID])

                        # mergelist[1].extend([-1, new_ind])
                        mergelist[2].extend([-1, -2])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID)

                        del NodeMap[str(key)]
                    elif check_rack(NID, base_rack) and not check_rack(NID2, base_rack) and cur_key[index] is True and \
                            cur_key[index + 1] is True:
                        mergelist[0].extend([NID, NID2])
                        mergelist[1].extend([NID2, NID])
                        # mergelist[1].extend([new_ind, -1])
                        mergelist[2].extend([-2, -1])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID2)
                        del NodeMap[str(mykeys[index + 1])]

        # print(len(NodeMap))
        mykeys = list(NodeMap.keys())
        mykeys.sort(key=float)
        # print(mykeys)

        cur_key = [True] * len(mykeys)
        for index, key in enumerate(mykeys):
            if index < len(mykeys) - 1:
                if str(key) in NodeMap and str(mykeys[index + 1]) in NodeMap:

                    NID = NodeMap[str(key)]
                    NID2 = NodeMap[str(mykeys[index + 1])]
                    if check_rack(NID, base_rack) and check_rack(NID2, base_rack) and cur_key[index] is True and \
                            cur_key[
                                index + 1] is True:
                        mergelist[0].extend([NID, NID2])

                        mergelist[1].extend([NID2, NID])
                        # mergelist[1].extend([-1, new_ind])
                        mergelist[2].extend([-1, -2])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID)
                        del NodeMap[str(key)]

    return mergelist
    # print(len(NodeMap))


# Convert to NID to rank_number
def post(mergelist, RankMap):  # , NodeMap):
    for i in range(len(mergelist[0])):
        mergelist[0][i] = RankMap[mergelist[0][i]]
        mergelist[1][i] = RankMap[mergelist[1][i]]

    # for index, item in enumerate(mergelist[0]):
    #    mergelist[0][index] = RankMap[item]
    #    mergelist[1][index] = RankMap[mergelist[1][index]]

    for index, item in enumerate(mergelist[2]):
        if item == -2 and index + 1 < len(mergelist[0]):
            ind = mergelist[0][index + 1::].index(mergelist[0][index])
            mergelist[2][index] = ind + index + 1

        # elif item == -2 and index % 2 == 0 and index+1<len(mergelist[0]):
        #    ind = mergelist[0][index+1::].index(item)
        #    mergelist[2][index] = ind+index+1

    # Sort first
    # rank_num = len(RankMap)
    # depths = list(NodeMap.keys()).map()
    # inv_map = {k: RankMap[v] for k, v in NodeMap.items()}

    # outlist = []

    # X = mergelist[0][0:rank_num]
    # Y = mergelist[1][0:rank_num]

    # Y2 = [x for _, x in sorted(zip(X, Y), key=lambda pair: RankMap[pair[0]])]
    # X2 = [x for x, _ in sorted(zip(X, Y), key=lambda pair: RankMap[pair[0]])]

    # Rsort = sorted(R2sort, key=lambda nid: inv_map[nid])
    # print(X2+mergelist[0][rank_num::])
    # outlist.append(X2+mergelist[0][rank_num::])
    # outlist.append(Y2+mergelist[1][rank_num::])

    # NID to Rank_Num
    # print(outlist)
    # for index, NID in enumerate(outlist[0]):
    #    outlist[0][index] = RankMap[NID]

    # return outlist


# CLI
def create():
    import argparse
    import csv
    p = argparse.ArgumentParser()
    p.add_argument('-n', '--num', default=64, type=int, dest='num', help='Number of Images')
    # p.add_argument('--csv', help='save as csv file')
    p.add_argument('-o', '--out', help='output file')
    p.add_argument('-f', '--file', help='input file')
    p.add_argument('-i', '--input', nargs='*', help='input string')

    ns = p.parse_args()

    image_num = int(ns.num)

    if ns.input is not None:
        [NodeMap, Groups, RankMap] = pre_processing2(joblist=ns.input)
    elif ns.file is not None:
        [NodeMap, Groups, RankMap] = pre_processing2(inputfile=ns.file)
    else:
        [NodeMap, Groups, RankMap] = pre_processing(image_num)

    # NodeMap2 = {**NodeMap}
    # print(NodeMap)
    # print(RankMap)
    base_rack = find_base(Groups)
    mergelist = merge(NodeMap, base_rack)
    # print(mergelist)
    # mlist = post(mergelist,RankMap, NodeMap2)
    post(mergelist, RankMap)

    # print(mergelist)
    tlist = list(map(list, zip(*mergelist)))

    if ns.out is not None:
        filename = "{}.csv".format(ns.out)
        with open(filename, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerows(tlist)


if __name__ == '__main__':
    # pass
    create()
