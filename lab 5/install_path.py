#!/usr/bin/python

import sys
import re # For regex

import ryu_ofctl
from ryu_ofctl import *

def main(macHostA, macHostB):
    print "Instaling flows for %s <==> %s" %(macHostA, macHostB) 

    ##### FEEL FREE TO MODIFY ANYTHING HERE #####
    try:
        pathA2B = dijkstras(macHostA, macHostB)
        installPathFlows(macHostA, macHostB, pathA2B)
    except:
        raise
    return 0


#Installs end-to-end bi-directional flows in all switches
def installPathFlows(macHostA, macHostB, pathA2B):
    for node in pathA2B:
        flow1 = FlowEntry()
        flow2 = FlowEntry()
        act1 = OutputAction(node['out_port'])
        act2 = OutputAction(node['in_port'])
        flow1.in_port = node['in_port']
        flow2.in_port = node['out_port']
        flow1.addAction(act1)
        flow2.addAction(act2)
        dpid = node['dpid']
        insertFlow(dpid, flow1)
        insertFlow(dpid, flow2)
    return

def portsFromDpid (from_dpid, to_dpid):
    points = listSwitchLinks(from_dpid).get('links')
    for p in points:
        if (p['endpoint1']['dpid'] == to_dpid):
            return (p['endpoint2']['port'], p['endpoint1']['port'])
        elif (p['endpoint2']['dpid'] == to_dpid):
            return (p['endpoint1']['port'], p['endpoint2']['port'])
    return None

# Returns List of neighbouring DPIDs
def findNeighbours (dpid):
    if type(dpid) not in (int, long) or dpid < 0:
        raise TypeError ("DPID should be a positive integer value")

    neighbours = []

    ##### YOUR CODE HERE #####
    points = listSwitchLinks(dpid).get('links')
    for p in points:
        if (p['endpoint1']['dpid'] == dpid):
            neighbours.append(p['endpoint2'])
        else:
            neighbours.append(p['endpoint1'])
    return neighbours

def dijkstras(macHostA, macHostB):
    def nodeDict(dpid, in_port, out_port):
        assert type(dpid) in (int, long)
        assert type(in_port) is int
        assert type(out_port) is int
        return {'dpid': dpid, 'in_port': in_port, 'out_port': out_port}

    INFINITY = float('inf')
    setOfAllNodes = listSwitches().get('dpids')
    distanceFromA = {}
    parents = {}
    leastDistNeighbour = {}
    pathAtoB = []

    A = getMacIngressPort(macHostA)
    B = getMacIngressPort(macHostB)

    if A.get('dpid') == None or B.get('dpid') == None:
        raise (Exception)

    currDpid = A['dpid']
    
    for node in setOfAllNodes:
        distanceFromA[node] = INFINITY
    
    distanceFromA[currDpid] = 0
    setOfAllNodes.remove(currDpid)
    setOfAllNodes.insert(0, currDpid)

    while (setOfAllNodes):
        currDpid = setOfAllNodes.pop(0)
        for neighbour in findNeighbours(currDpid):
            if (distanceFromA[neighbour['dpid']] > distanceFromA[currDpid] + 1):
                distanceFromA[neighbour['dpid']] = distanceFromA[currDpid] + 1
                parents[neighbour['dpid']] = currDpid
    
    currDpid = B['dpid']
    out_port = B['port']

    while currDpid != A['dpid']:
        in_port = portsFromDpid (parents[currDpid], currDpid)
        pathAtoB.append(nodeDict(currDpid, in_port[1], out_port))
        currDpid = parents[currDpid]
        out_port = in_port[0]
    
    pathAtoB.append(nodeDict(A['dpid'], A['port'], out_port))
    pathAtoB.reverse()

    return pathAtoB




