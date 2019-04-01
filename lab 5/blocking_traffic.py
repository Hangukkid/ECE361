import ryu_ofctl as ryu

dpid = 1
ryu.deleteAllFlows(dpid)

flow = ryu.FlowEntry()
act1 = ryu.OutputAction(2)

flow.in_port = 3
flow.addAction(act1)
ryu.insertFlow(dpid, flow)

flow2 = ryu.FlowEntry()
act2 = ryu.OutputAction(2)

flow2.in_port = 3
flow2.addAction(act2)
ryu.insertFlow(dpid, flow2)