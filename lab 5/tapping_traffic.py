import ryu_ofctl as ryu

dpid = 1
ryu.deleteAllFlows(dpid)

flow = ryu.FlowEntry()
act1 = ryu.OutputAction(1)
act2 = ryu.OutputAction(2)

flow.in_port = 3
flow.addAction(act1)
flow.addAction(act2)
ryu.insertFlow(dpid, flow)

flow2 = ryu.FlowEntry()
act3 = ryu.OutputAction(3)
act4 = ryu.OutputAction(2)

flow2.in_port = 1
flow2.addAction(act3)
flow2.addAction(act4)
ryu.insertFlow(dpid, flow2)