# def get_ID():
#     print("This returns unique ID for a node in format of Cxxx-yyz")
#
#
# def decode(uniqueID):
#     print("Decode the nodeID to internal uniqueID")
#
#
# def encode(internalID):
#     print("Encode internal ID to a unique ID")
#
#
# def get_data(uniqueID):
#     print("Return Image Object {ID:None, depth: value}")
#
#
# def get_location(uniqueID):
#     # Rack ID, which half rack, node number
#     location = [int(uniqueID[1:4]), int(uniqueID[5:7]) > 6, int(uniqueID[7])]
#     return location
#
#
# def get_Input(IDs):
#     initial = []
#     for ID in IDs:
#         initial.append({"ID": decode(ID), "depth": get_data(ID)["depth"]})

# Final return list of nodes, target: Unique node ID (Cxxx-yyz) root: -1, index: whether sender/receiver, whereis parent

def addNode(Nodes, nodeval=None, imageDepth=None, halfrackNum=None, rackID=None):
    node = {"ID": "C" + '{:03}'.format(rackID) + "-" + '{:02}'.format(halfrackNum) + str(nodeval), "IMG": {}};
    node["IMG"]["data"] = {}
    node["IMG"]["depth"] = imageDepth
    Nodes.append(node)
    return node["ID"]


image_num = 64


def pre_processing(image_num):
    import random
    import math
    depths = list(range(image_num))
    nodes = []
    random.shuffle(depths)
    NodeMap = {}
    Groups = {}
    d = 0
    rack_num = math.ceil(image_num / 56)
    for r in range(rack_num):
        cur_rack = random.randint(0, 999)
        for i in range(14):
            for j in range(4):
                if d >= image_num:
                    return [NodeMap, Groups]
                else:
                    NID = addNode(nodes, nodeval=j, imageDepth=depths[d], halfrackNum=i, rackID=cur_rack)
                    NodeMap[str(depths[d])] = NID
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


def check_rack(NID, base_rack):
    group = NID[0:4] + str(int(int(NID[5:7]) > 6))
    if group == base_rack:
        return True
    else:
        return False


def find_base(Groups):
    import operator
    base_rack = max(Groups.items(), key=operator.itemgetter(1))[0]
    return base_rack


def merge(NodeMap, base_rack):
    mergelist = [[], []]
    mykeys = list(range(len(NodeMap)))
    new_ind = len(NodeMap)
    while len(mykeys) > 1:
        mykeys = list(NodeMap.keys())
        mykeys.sort(key=int)
        # print(mykeys)
        cur_key = [True] * len(mykeys)
        for index, key in enumerate(mykeys):
            if index < len(mykeys) - 1:
                if str(key) in NodeMap and str(mykeys[index + 1]) in NodeMap:
                    NID = NodeMap[str(key)]
                    NID2 = NodeMap[str(mykeys[index + 1])]
                    if not check_rack(NID, base_rack) and check_rack(NID2, base_rack) and cur_key[index] is True and \
                            cur_key[index + 1] is True:
                        mergelist[0].extend([NID2, NID])
                        mergelist[1].extend([-1, new_ind])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID)

                        del NodeMap[str(key)]
                    elif check_rack(NID, base_rack) and not check_rack(NID2, base_rack) and cur_key[index] is True and \
                            cur_key[index + 1] is True:
                        mergelist[0].extend([NID2, NID])
                        mergelist[1].extend([new_ind, -1])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID2)
                        del NodeMap[str(mykeys[index + 1])]

        # print(len(NodeMap))
        mykeys = list(NodeMap.keys())
        mykeys.sort(key=int)
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
                        mergelist[0].extend([NID2, NID])
                        mergelist[1].extend([-1, new_ind])
                        new_ind = new_ind + 1
                        cur_key[index] = False
                        cur_key[index + 1] = False
                        # print(NID)
                        del NodeMap[str(key)]

    return mergelist
    # print(len(NodeMap))


def create():
    import argparse
    import csv
    p = argparse.ArgumentParser()
    p.add_argument('-n', '--num', default=64, type=int, dest='num', help='Number of Images')
    # p.add_argument('--csv', help='save as csv file')
    p.add_argument('-o', '--out', help='output file')

    ns = p.parse_args()

    image_num = int(ns.num)
    [NodeMap, Groups] = pre_processing(image_num)
    base_rack = find_base(Groups)
    mergelist = merge(NodeMap, base_rack)


    tlist = list(map(list, zip(*mergelist)))

    if ns.out is not None:
        filename = "{}.csv".format(ns.out)
        with open(filename, "w", newline='') as f:
            writer = csv.writer(f)
            writer.writerows(tlist)


if __name__ == '__main__':
    # pass
    create()
