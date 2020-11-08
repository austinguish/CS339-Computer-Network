#http://docs.openvswitch.org/en/latest/faq/openflow/
#https://mail.openvswitch.org/pipermail/ovs-discuss/2016-August/042394.html
#https://stackoverflow.com/questions/36949861/group-table-issue-openflow-mininet

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ether_types
s1op = 1
s2op = 1

class SimpleSwitch13(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch13, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
    def add_flow_tmp(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,hard_timeout=5,
                                    flags=ofproto.OFPFF_SEND_FLOW_REM,priority=priority, match=match,
                                    instructions=inst)
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,hard_timeout=5,
                                    flags=ofproto.OFPFF_SEND_FLOW_REM,match=match, instructions=inst)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        global s1op
        global s2op
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        # install table-miss flow entry
        '''
        match = parser.OFPMatch()
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 0, match, actions)
        '''
        # switch s1
        if datapath.id == 1:
            
            #add the return flow for h1 in s1.  
            # h1 is connected to port 3.
            actions = [parser.OFPActionOutput(s1op)]
            match = parser.OFPMatch(in_port=3)
            self.add_flow_tmp(datapath, 10, match, actions)
            op = 2
            actions = [parser.OFPActionOutput(3)]
            match = parser.OFPMatch(in_port=1)
            self.add_flow(datapath, 10, match, actions)

            actions = [parser.OFPActionOutput(3)]
            match = parser.OFPMatch(in_port=2)
            self.add_flow(datapath, 10, match, actions)


        # switch s2
        if datapath.id == 2:
            actions = [parser.OFPActionOutput(s2op)]
            match = parser.OFPMatch(in_port=3)
            self.add_flow_tmp(datapath, 10, match, actions)
            op = 2
            #add the return flow for h2 in s2.  
            # h2 is connected to port 3.
            actions = [parser.OFPActionOutput(3)]
            match = parser.OFPMatch(in_port=1)
            self.add_flow(datapath, 10, match, actions)

            actions = [parser.OFPActionOutput(3)]
            match = parser.OFPMatch(in_port=2)
            self.add_flow(datapath, 10, match, actions)


        # switch s4
        if datapath.id == 4:
            
            actions = [parser.OFPActionOutput(2)]
            match = parser.OFPMatch(in_port=1)
            self.add_flow(datapath, 10, match, actions)

            actions = [parser.OFPActionOutput(1)]
            match = parser.OFPMatch(in_port=2)
            self.add_flow(datapath, 10, match, actions)


        # switch s3
        if datapath.id == 3:
            # h1 is connected to port 3.
            actions = [parser.OFPActionOutput(2)]
            match = parser.OFPMatch(in_port=1)
            self.add_flow(datapath, 10, match, actions)

            actions = [parser.OFPActionOutput(1)]
            match = parser.OFPMatch(in_port=2)
            self.add_flow(datapath, 10, match, actions)

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        datapath.send_msg(mod)


    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg .match['in_port']

        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]

        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            return
        dst = eth.dst
        src = eth.src

        dpid = datapath.id
        if dst[:5] == "33:3":
            self.logger.info("drop ipv6 multicast packet %s", dst)
            return

        self.mac_to_port.setdefault(dpid, {})

        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        if(dpid==1 or dpid == 4):
            if(op==1):
                out_port=2
                op = 2
            else:
                out_port = 1
                op = 1
        actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD:
            match = parser.OFPMatch(in_port=in_port, eth_dst=dst, eth_src=src)
            # verify if we have a valid buffer_id, if yes avoid to send both
            # flow_mod & packet_out
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow_tmp(datapath, 1, match, actions, msg.buffer_id)
                return
            else:
                self.add_flow_tmp(datapath, 1, match, actions)
        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)
    
    @set_ev_cls(ofp_event.EventOFPFlowRemoved, MAIN_DISPATCHER)
    def _flow_removed_handler(self, ev):
        global s1op
        global s2op
        msg = ev.msg
        dp = msg.datapath
        ofp = dp.ofproto
        parser = dp.ofproto_parser
        if msg.reason == ofp.OFPRR_IDLE_TIMEOUT:
            reason = 'IDLE TIMEOUT'
        elif msg.reason == ofp.OFPRR_HARD_TIMEOUT:
                reason = 'HARD TIMEOUT'
        elif msg.reason == ofp.OFPRR_DELETE:
            reason = 'DELETE'
        elif msg.reason == ofp.OFPRR_GROUP_DELETE:
            reason = 'GROUP DELETE'
        else:
            reason = 'unknown'

        self.logger.debug('OFPFlowRemoved received: ''datapath id=%s' 
                              'cookie=%d priority=%d reason=%s table_id=%d '
                              'duration_sec=%d duration_nsec=%d '
                              'idle_timeout=%d hard_timeout=%d '
                              'packet_count=%d byte_count=%d match.fields=%s',
                              dp.id,msg.cookie, msg.priority, reason, msg.table_id,
                              msg.duration_sec, msg.duration_nsec,
                              msg.idle_timeout, msg.hard_timeout,
                              msg.packet_count, msg.byte_count, msg.match)
        if(dp.id==1):
            if(s1op==1):
                s1op = 2
            else:
                s1op = 1
        print("s1op is "+str(s1op))
        actions = [parser.OFPActionOutput(s1op)]
        match = parser.OFPMatch(in_port=3)
        self.add_flow_tmp(dp, 10, match, actions)



