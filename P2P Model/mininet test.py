#!/usr/bin/python

"""
Simple example of setting network and CPU parameters
"""


from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSBridge
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel, info
from mininet.cli import CLI

from sys import argv

# It would be nice if we didn't have to do this:
# pylint: disable=arguments-differ

class SingleSwitchTopo( Topo ):
    def build( self ):
        tracker = self.addHost('tracker', cpu=.25)
        switch1 = self.addSwitch('s1',stp=True)
        host1 = self.addHost('h1', cpu=.25)
        host2 = self.addHost('h2', cpu=.25)
        host3 = self.addHost('h3', cpu=.25)
        host4 = self.addHost('h4', cpu=.25)
        host5 = self.addHost('h5', cpu=.25)
        host6 = self.addHost('h6', cpu=.25)
        self.addLink(tracker, switch1, delay='5ms',bw=1000,loss=0, use_htb=True)
        self.addLink(host1, switch1, delay='5ms',bw=1000, loss=0, use_htb=True)
        self.addLink(host2, switch1,  delay='5ms', bw=1000,loss=0, use_htb=True)
        self.addLink(host3, switch1,  delay='5ms',bw=1000, loss=0, use_htb=True)
        self.addLink(host4, switch1, delay='5ms', bw=1000,loss=0, use_htb=True)
        self.addLink(host5, switch1,  delay='5ms', bw=1000,loss=0, use_htb=True)
        self.addLink(host6, switch1,  delay='5ms', bw=1000,loss=0, use_htb=True)

def Test():
    "Create network and run simple performance test"
    topo = SingleSwitchTopo()
    net = Mininet( topo=topo,
                   host=CPULimitedHost, link=TCLink,
                   autoStaticArp=False )
    net.start()
    info( "Dumping host connections\n" )
    dumpNodeConnections(net.hosts)
    tracker,h1,h2,h3,h4,h5,h6 = net.getNodeByName('tracker','h1','h2','h3','h4','h5','h6')
    CLI(net,script = "script")
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    # Prevent test_simpleperf from failing due to packet loss
    Test()
