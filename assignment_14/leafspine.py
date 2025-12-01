from mininet.topo import Topo

class LeafSpine(Topo):
    "Simple Leaf-Spine topology for Assignment 14"

    def build(self):
        # 1. Add Spine Switches (The Core Backbone)
        spine1 = self.addSwitch('s1')
        spine2 = self.addSwitch('s2')

        # 2. Add Leaf Switches (Access Layer)
        leaf1 = self.addSwitch('s3')
        leaf2 = self.addSwitch('s4')

        # 3. Add Hosts (The Servers/Computers)
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')

        # 4. Connect Spines to Leaves (Full Mesh)
        # Every Spine connects to every Leaf
        self.addLink(spine1, leaf1)
        self.addLink(spine1, leaf2)
        self.addLink(spine2, leaf1)
        self.addLink(spine2, leaf2)

        # 5. Connect Leaves to Hosts
        # Hosts only connect to Leaf switches
        self.addLink(leaf1, h1)
        self.addLink(leaf2, h2)

topos = { 'leafspine': ( lambda: LeafSpine() ) }
