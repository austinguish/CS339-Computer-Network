 #!/usr/bin/python
"""
Simple example of setting network and CPU parameters  
"""
import threading
import time
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSBridge
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import quietRun, dumpNodeConnections
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from sys import argv
import time
from threading import Thread

"""重新定义带返回值的线程类"""


class MyThread(Thread):
    def __init__(self, func, kargs):
        super(MyThread, self).__init__()
        self.func = func
        self.kargs = kargs

    def run(self):
        self.result = self.func(**self.kargs)

    def get_result(self):
        try:
            return self.result
        except Exception:
            return None
# It would be nice if we didn't have to do this:
# pylint: disable=arguments-differ
class SingleSwitchTopo( Topo ):
    def build( self ):
        switch1 = self.addSwitch('s1')
        switch2 = self.addSwitch('s2')
        host1 = self.addHost('h1', cpu=.25)
        host2 = self.addHost('h2', cpu=.25)
        host3 = self.addHost('h3', cpu=.25)
        host4 = self.addHost('h4', cpu=.25)
        host5 = self.addHost('h5', cpu=.25)
        host6 = self.addHost('h6', cpu=.25)
        host7 = self.addHost('h7', cpu=.25)
        host8 = self.addHost('h8', cpu=.25)
        host9 = self.addHost('h9', cpu=.25)
        host10 = self.addHost('h10', cpu=.25)
        self.addLink(host1, switch1, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host2, switch1, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(switch1, switch2, bw=100, delay='5ms', loss=0.1, use_htb=True)
        self.addLink(host3, switch1, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host4, switch1, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host5, switch1, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host6, switch2, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host7, switch2, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host8, switch2, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host9, switch2, bw=100, delay='5ms', loss=0, use_htb=True)
        self.addLink(host10, switch2, bw=100, delay='5ms', loss=0, use_htb=True)
def Test(tcp):
    "Create network and run simple performance test"
    topo = SingleSwitchTopo()
    net = Mininet( topo=topo,
                   host=CPULimitedHost, link=TCLink,
                   autoStaticArp=False )
    net.start()
    info( "Dumping host connections\n" )
    dumpNodeConnections(net.hosts)
    # set up tcp congestion control algorithm
    output = quietRun( 'sysctl -w net.ipv4.tcp_congestion_control=' + tcp )
    assert tcp in output
    info( "Testing bandwidth between h1 and h4 under TCP " + tcp + "\n" )
    h1,h2,h3,h4,h5,h6,h7,h8,h9,h10 = net.getNodeByName('h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'h7', 'h8', 'h9', 'h10')
    t1 = MyThread(net.iperf, kargs={"hosts":[h1,h6],"seconds":10})
    t2 = MyThread(net.iperf, kargs={"hosts":[h2,h7],"seconds":10})
    t3 = MyThread(net.iperf, kargs={"hosts":[h3,h8],"seconds":10})
    t4 = MyThread(net.iperf, kargs={"hosts":[h4,h9],"seconds":10})
    t5 = MyThread(net.iperf, kargs={"hosts":[h5,h10],"seconds":10})
    t1.start()
    t2.start()
    t3.start()
    t4.start()
    t5.start()
    t1.join()
    t2.join()
    t3.join()
    t4.join()
    t5.join()
    res1 = t1.get_result()
    res2 = t2.get_result()
    res3 = t3.get_result()
    res4 = t4.get_result()
    res5 = t5.get_result()
    #_serverbw, clientbw = net.iperf( [ h1, h2 ], seconds=10 )
    info( res1, '\n' )
    info( res2, '\n' )
    info( res3, '\n' )
    info( res4, '\n' )
    info( res5, '\n' )
    CLI(net)
    net.stop()
if __name__ == '__main__':
    setLogLevel('info')
    # pick a congestion control algorithm, for example, 'reno', 'cubic', 'bbr', 'vegas', 'hybla', etc.
    tcp = 'reno'
    Test(tcp)
