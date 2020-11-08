#!/usr/bin/python

'''

              Switch3 
            /    |     \                   
h1 ---Switch1    |     Switch2-----h2
            \    |     /
              Switch4

'''




from mininet.topo import Topo
from mininet.net import Mininet
from mininet.log import setLogLevel
from mininet.cli import CLI
from mininet.node import OVSSwitch, Controller, RemoteController
from time import sleep


class SingleSwitchTopo(Topo):
    "Single switch connected to n hosts."
    def build(self):
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')

        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        self.addLink(s1,s4,1,1)
        self.addLink(s1,s3,2,1) 
        self.addLink(s3,s4,3,3)
        self.addLink(s4,s2,2,2)
        self.addLink(s2,s3,1,2)
        self.addLink(s1,h1,3,1)
        self.addLink(s2,h2,3,1)

if __name__ == '__main__':
    setLogLevel('info')
    topo = SingleSwitchTopo()
    c1 = RemoteController('c1', ip='127.0.0.1')
    net = Mininet(topo=topo, controller=c1)
    net.start()
    CLI(net)
    net.stop() 
