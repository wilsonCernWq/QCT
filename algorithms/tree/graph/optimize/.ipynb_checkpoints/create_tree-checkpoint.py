def get_ID():
    print("This returns unique ID for a node in format of Cxxx-yyz")


def decode(uniqueID):
    print("Decode the nodeID to internal uniqueID")


def encode(internalID):
    print("Encode internal ID to a unique ID")


def get_data(uniqueID):
    print("Return Image Object {ID:None, depth: value}")


def get_location(uniqueID):
    # Rack ID, which half rack, node number
    location = [int(uniqueID[1:4]), int(uniqueID[5:7]) > 6, int(uniqueID[7])]
    return location


def get_Input(IDs):
    initial = []
    for ID in IDs:
        initial.append({"ID": decode(ID), "depth": get_data(ID)["depth"]})

# Final return list of nodes, target: Unique node ID (Cxxx-yyz) root: -1, index: whether sender/receiver, whereis parent
