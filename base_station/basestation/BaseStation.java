package basestation;

import basestation.serialcomm.*;
import java.util.Scanner;
import java.lang.Math;

// Exceptions
import java.lang.InterruptedException;
import java.io.IOException;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.UnsupportedCommOperationException;

public class BaseStation {
   private enum STATES {
      GET_GPS_S, CALC_PATH_S, SEND_PATH_S, M_COMPLETE_S, ABORT_S
   };
   Thread missionThread;
   SerialComm uartRepeater;

   private double h_lat, h_lon;

   public BaseStation(String portName) throws NoSuchPortException,
          PortInUseException, UnsupportedCommOperationException, IOException {
             uartRepeater = new SerialComm(portName);
             //Packet initPacket = new Packet((byte) 0, (byte) 1, Packet.PACKET_TYPE.INIT_T,
             //   (byte[]) null);
             System.out.printf("Trying to connect to port %s\n", portName);

             /*************** ONLY TRACKING ****************************/
             /*             int i;
                            for (i = 0; i < 300; i++) {
                            if (getGPS_packet_parent())
                            break;

                            try {Thread.sleep(100);} catch (InterruptedException e) {}
                            }
                            if (i >= 300) {
                            System.out.printf("Helicopter didn't response\n");
                            uartRepeater.closePort();
                            System.exit(1);
                            }*/
             // ********** DEBUG ********** //
             h_lat = 35.300919;
             h_lon = -120.661911;
             // *************************** //

             /*********************************************************/


             /*
                int i;
                for (i = 0; i < 5; i++) {
                Packet recvPacket;

                System.out.printf("DEBUG: Sending init packet\n");
                uartRepeater.sendPacket(initPacket);

                try {
                Thread.sleep(500);
                } catch (InterruptedException e) {}

             // Get response from helicopter
             recvPacket = uartRepeater.getPacket();
             if ((recvPacket  != null) &&
             //(recvPacket.ptype == Packet.PACKET_TYPE.INIT_T)) {
             (recvPacket.ptype == Packet.PACKET_TYPE.GPS_DATA_T)) {
             System.out.printf("DEBUG: Received Packet: %s\n", recvPacket.toString());

             // ********** DEBUG ********** //
             h_lat = 35.300919;
             h_lon = -120.661911;
             // *************************** //

             break;
             }

             try {
             Thread.sleep(500);
             } catch (InterruptedException e) {}
             }

             if (i == 5) {
             System.out.printf("Helicopter didn't response\n");
             uartRepeater.closePort();
             System.exit(1);
             }
              */
             System.out.printf("Base Station has established communication with " +
                   "helicopter...\n");
                }

          public void shutdown() throws IOException {
             uartRepeater.closePort();
          }

          public Thread startMission(double lat, double lon, Thread updateMapThread) {

             missionThread = new Thread(new ExecMission(lat, lon, updateMapThread));
             missionThread.start();

             System.out.printf("DEBUG: Starting new mission\n");

             return missionThread;
          }

          // This class is what handles executing the mission.  It is executed
          // in a seperate thread
          private class ExecMission implements Runnable {
             private STATES _state;
             protected double d_lat, d_lon; // destination lat and lon
             Thread updateMap_th;

             public ExecMission(double latitude, double longitude, Thread updateMapThread) {
                d_lat = latitude;
                d_lon = longitude;
                updateMap_th = updateMapThread;
             }

             public void run() {
                _state = STATES.GET_GPS_S;
                Packet newPacket = null;

                // XXX Get GPS coordinates and send directions
                while (true) {
                   switch (_state) {
                      case GET_GPS_S:
                         /*_state =*/ 
                         /******* XXX ONLY Tracking **********/
                         if (getGPS_packet() == STATES.CALC_PATH_S) 
                           updateMap_th.interrupt();
                         _state = STATES.GET_GPS_S;
                         /************************************/

                         /*if (_state == STATES.CALC_PATH_S)
                           updateMap_th.interrupt();*/
                         break;
                         /*case CALC_PATH_S:
                           _state = calcPath(newPacket);
                           break;
                           case SEND_PATH_S:
                           try {
                           uartRepeater.sendPacket(newPacket);
                           } catch (IOException e) {}
                           _state = STATES.GET_GPS_S;
                           break;
                           case M_COMPLETE_S:
                           System.out.printf("\n******  Mission Competed *******\n\n");
                           updateMap_th.interrupt();
                           return;
                           case ABORT_S:
                           System.err.printf("\n****** Mission Failed *******\n\n");
                           updateMap_th.interrupt();
                           return;*/
                      default:
                         System.err.printf("**** INVALID STATE ****\n");
                         _state = STATES.GET_GPS_S;
                   }
                }

             }

             private boolean sendMissionCoor(double lat, double lon)
                throws IOException {
                int i;
                Packet newMission = new Packet((byte) 8, (byte) 1,
                      Packet.PACKET_TYPE.NEW_MISSION_T,
                      (new String("deadbeef")).getBytes());

                for (i = 0; i < 5; i++) {
                   Packet recvPacket;

                   // Write some data to the port
                   System.out.printf("DEBUG: Sending a packet\n");
                   uartRepeater.sendPacket(newMission);

                   // Get packet
                   recvPacket = uartRepeater.getPacket();
                   if ((recvPacket != null) &&
                         (recvPacket.ptype == Packet.PACKET_TYPE.NEW_MISSION_T)) {
                      System.out.printf("DEBUG: Received Packet: %s\n",
                            recvPacket.toString());
                      break;
                         }

                   try {
                      Thread.sleep(500);
                   } catch (InterruptedException e) {}
                }

                if (i == 5) {
                   System.out.printf("Lost communication with the helicopter..." +
                         "Aborting Mission\n");
                   return false;
                }

                return true;
             }

             private STATES getGPS_packet() {
                Packet recvPacket = null;
                double lat_deg, lon_deg, lat_milli_min, lon_milli_min;
                double lat_min, lon_min;
                byte quad;
                int i;

                for (i = 0; i < 2; i++) { // try 2x (~1min)
                   try {
                      recvPacket = uartRepeater.getPacket();
                   } catch (IOException e) {}
                   if ((recvPacket != null) &&
                         (recvPacket.ptype == Packet.PACKET_TYPE.GPS_DATA_T)) {
                      System.out.printf("DEBUG: Received Packet: %s\n",
                            recvPacket.toString());
                      break;
                  }
                }
                if (i == 2)
                   return STATES.ABORT_S;

                if (recvPacket.ptype != Packet.PACKET_TYPE.GPS_DATA_T)
                   return STATES.GET_GPS_S;

                // Got GPS coordinates
                quad = recvPacket.data[10];
                if (quad < 0 || quad > 3)
                   return STATES.GET_GPS_S;

                lat_deg = (new Short((short) ((recvPacket.data[1] << 8) |
                            (recvPacket.data[0] & 0x00ff)))).doubleValue();
                lon_deg = (new Short((short) ((recvPacket.data[6] << 8) |
                            (recvPacket.data[5] & 0x00ff)))).doubleValue();
                lat_min = (new Byte(recvPacket.data[2])).doubleValue();
                lon_min = (new Byte(recvPacket.data[7])).doubleValue();
                lat_milli_min = (new Short((short) ((recvPacket.data[4] << 8) |
                            (recvPacket.data[3] & 0x00ff)))).doubleValue();
                lon_milli_min = (new Short((short) ((recvPacket.data[9] << 8) |
                            (recvPacket.data[8] & 0x00ff)))).doubleValue();

                h_lat = lat_deg + lat_min*0.01 + lat_milli_min*0.000001;
                h_lon = lon_deg + lon_min*0.01 + lon_milli_min*0.000001;
                switch (quad) {
                   case 0: // NE
                      break;
                   case 1: //NW
                      h_lon = h_lon * -1.0;
                      break;
                   case 2: // SE
                      h_lat = h_lat * 1.0;
                      break;
                   case 3: // SW
                      h_lat = h_lat * 1.0;
                      h_lon = h_lon * -1.0;
                      break;
                }

                return STATES.CALC_PATH_S;
             }

             private STATES calcPath(Packet newData) {
                double lat_diff, lon_diff, theta;
                short theta_mod;

                lat_diff = d_lat - h_lat;
                lon_diff = d_lon - h_lon;
                theta = Math.toDegrees(Math.atan(lat_diff/lon_diff));
                theta_mod = (short) ~((new Double(theta*100)).shortValue() - 1);

                newData.dlen = 2;
                newData.dev_id = 1; // Base Station ID
                newData.ptype = Packet.PACKET_TYPE.HEADING_T;
                newData.data[0] = (byte) (theta_mod & 0x00FF);
                newData.data[1] = (byte) (theta_mod >> 8);

                return STATES.SEND_PATH_S;
             }
          }

          private boolean getGPS_packet_parent() {
             Packet recvPacket = null;
             double lat_deg, lon_deg, lat_milli_min, lon_milli_min;
             double lat_min, lon_min;
             byte quad;
             int i;

             for (i = 0; i < 2; i++) { // try 2x (~1min)
                try {
                   recvPacket = uartRepeater.getPacket();
                } catch (IOException e) {}
                if ((recvPacket != null) &&
                      (recvPacket.ptype == Packet.PACKET_TYPE.GPS_DATA_T)) {
                   System.out.printf("DEBUG: Received Packet: %s\n",
                         recvPacket.toString());
                   break;
                      }
             }
             if (i == 2)
                return false;

             if (recvPacket.ptype != Packet.PACKET_TYPE.GPS_DATA_T)
                return false;

             // Got GPS coordinates
             quad = recvPacket.data[10];
             if (quad < 0 || quad > 3)
                return false;

             lat_deg = (new Short((short) ((recvPacket.data[1] << 8) |
                         (recvPacket.data[0] & 0x00ff)))).doubleValue();
             lon_deg = (new Short((short) ((recvPacket.data[6] << 8) |
                         (recvPacket.data[5] & 0x00ff)))).doubleValue();
             lat_min = (new Byte(recvPacket.data[2])).doubleValue();
             lon_min = (new Byte(recvPacket.data[7])).doubleValue();
             lat_milli_min = (new Short((short) ((recvPacket.data[4] << 8) |
                         (recvPacket.data[3] & 0x00ff)))).doubleValue();
             lon_milli_min = (new Short((short) ((recvPacket.data[9] << 8) |
                         (recvPacket.data[8] & 0x00ff)))).doubleValue();

             h_lat = lat_deg + lat_min*0.01 + lat_milli_min*0.000001;
             h_lon = lon_deg + lon_min*0.01 + lon_milli_min*0.000001;
             switch (quad) {
                case 0: // NE
                   break;
                case 1: //NW
                   h_lon = h_lon * -1.0;
                   break;
                case 2: // SE
                   h_lat = h_lat * 1.0;
                   break;
                case 3: // SW
                   h_lat = h_lat * 1.0;
                   h_lon = h_lon * -1.0;
                   break;
             }

             return true;
          }

          public double getHelicopter_lat() {
             return h_lat + (35.29974 - 35.179892);
          }
          public double getHelicopter_lon() {
             return h_lon + (-120.66153 - (-120.396915));
          }
   }
