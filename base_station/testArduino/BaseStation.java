
import serialcomm.*;
import java.util.Scanner;

// Exceptions
import java.lang.InterruptedException;
import java.io.IOException;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.UnsupportedCommOperationException;

public class BaseStation {
   SerialComm uartRepeater;

   public BaseStation(String portName) throws NoSuchPortException,
         PortInUseException, UnsupportedCommOperationException, IOException {
      uartRepeater = new SerialComm(portName);

      System.out.printf("Base Station has established communication with " + 
         "helicopter...\n");
   }

   public static void main(String argv[]) throws NoSuchPortException,
          IOException, PortInUseException, UnsupportedCommOperationException {
      BaseStation thisStation = new BaseStation(argv[0]);
      Packet recvPacket;
      Packet initPacket = new Packet((byte) 0, (byte) 1, Packet.PACKET_TYPE.INIT_T,
         (byte[]) null);

      //for (int i = 0; i < 10000; i++) {
      while (true) {
         recvPacket = thisStation.uartRepeater.getPacket();
         if (recvPacket != null) 
            System.out.printf("DEBUG: Received Packet: %s\n", recvPacket.toString());
         
//         try { Thread.sleep(2000); } catch (Exception e) {}
//         thisStation.uartRepeater.sendPacket(initPacket);
      }
      
      //thisStation.shutdown();
      //System.out.printf("Goodbye :)\n");
   }

   public void shutdown() throws IOException {
      uartRepeater.closePort();
   }
}
