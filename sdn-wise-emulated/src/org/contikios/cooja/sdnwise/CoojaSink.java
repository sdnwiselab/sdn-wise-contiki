/*
 * Copyright (C) 2015 SDN-WISE
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package org.contikios.cooja.sdnwise;;

import com.github.sdnwiselab.sdnwise.mote.battery.SinkBattery;
import com.github.sdnwiselab.sdnwise.mote.core.*;
import com.github.sdnwiselab.sdnwise.packet.NetworkPacket;
import com.github.sdnwiselab.sdnwise.util.NodeAddress;
import java.io.*;
import java.net.*;
import java.util.logging.*;
import javax.swing.JOptionPane;
import org.contikios.cooja.*;

/**
 * @author Sebastiano Milardo
 */
public class CoojaSink extends AbstractCoojaMote {

    private Socket tcpSocket;
    private DataInputStream riceviOBJ;
    private DataOutputStream inviaOBJ;
    private InetAddress addrController;
    private int portController;

    public CoojaSink() {
        super();
    }

    public CoojaSink(MoteType moteType, Simulation simulation) {
        super(moteType, simulation);
    }

    @Override
    public final void init() {
        try {
            battery = new SinkBattery();
            String[] tmp = getControllerIpPort();

            addrController = InetAddress.getByName(tmp[0]);
            portController = Integer.parseInt(tmp[1]);

            core = new SinkCore((byte) 1,
                    new NodeAddress(this.getID()),
                    battery,
                    "00000001",
                    "00:01:02:03:04:05",
                    1,
                    addrController,
                    portController);
            core.start();
            startThreads();
        } catch (UnknownHostException ex) {
            log(ex.getLocalizedMessage());
        }
    }

    private class TcpListener implements Runnable {

        @Override
        public void run() {
            try {
                riceviOBJ = new DataInputStream(tcpSocket.getInputStream());
                while (true) {
                    NetworkPacket np = new NetworkPacket(riceviOBJ);
                    core.rxRadioPacket(np, 255);
                }
            } catch (IOException ex) {
                Logger.getLogger(CoojaSink.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    private class TcpSender implements Runnable {

        @Override
        public void run() {
            try {
                inviaOBJ = new DataOutputStream(tcpSocket.getOutputStream());
                while (true) {
                    NetworkPacket np = ((SinkCore) core).getControllerPacketTobeSend();
                    inviaOBJ.write(np.toByteArray());
                }
            } catch (IOException | InterruptedException ex) {
                Logger.getLogger(CoojaSink.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    @Override
    void startThreads() {
        super.startThreads();
        try {
            tcpSocket = new Socket(addrController, portController);
            new Thread(new TcpListener()).start();
            new Thread(new TcpSender()).start();
        } catch (IOException ex) {
            log(ex.getLocalizedMessage() + " " + addrController + ":" + portController);
        }

    }

    private String[] getControllerIpPort() {
        String s = (String) JOptionPane.showInputDialog(null,
                "Please insert the IP address and TCP port of the controller:",
                "SDN-WISE Sink",
                JOptionPane.QUESTION_MESSAGE, null, null, "192.168.1.101:9999");

        String[] tmp = s.split(":");

        if (tmp.length != 2) {
            return getControllerIpPort();
        } else {
            return tmp;
        }
    }
}
