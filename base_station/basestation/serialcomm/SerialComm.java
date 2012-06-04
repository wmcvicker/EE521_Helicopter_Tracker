package basestation.serialcomm;

import java.lang.Thread;
import java.lang.Byte;
import java.lang.InterruptedException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

import gnu.io.CommPortIdentifier;
import gnu.io.CommPort;
import gnu.io.SerialPort;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.UnsupportedCommOperationException;

public class SerialComm {
   public static final int BAUD = 9600;
   public static final int SERIAL_TIMEOUT = 30;
   protected InputStream is;
   protected OutputStream os;

   CommPortIdentifier portID;
   CommPort thePort;
   SerialPort myPort;

   // Constructor
   public SerialComm(String portName) throws NoSuchPortException,
         PortInUseException, UnsupportedCommOperationException, IOException {

      // Retrieve the port by name
      portID = CommPortIdentifier.getPortIdentifier(portName);
      if (portID.isCurrentlyOwned()) {
         System.out.printf("Error: Port is currently in use\n");
         System.exit(1);
      }
      thePort = portID.open("Autocopter Base Station", SERIAL_TIMEOUT * 1000);

      // Configure the serial port
      if (thePort instanceof SerialPort) {
         myPort = (SerialPort) thePort;
         myPort.setSerialPortParams(BAUD, SerialPort.DATABITS_8,
               SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

         myPort.enableReceiveTimeout(500);
         is = myPort.getInputStream();
         os = myPort.getOutputStream();
         System.out.printf("Successfully opened the port: %s\n", portName);
      } else {
         System.out.printf("Error: the input port %s is not a serial port\n",
               portName);
      }
   }

   public void closePort()
      throws IOException {

      if (is != null)
         is.close();

      os.flush();
      os.close();
      thePort.close();
   }


   private boolean getPreambles() throws IOException {
      byte preamble;

      while ((preamble = (byte) (this.is.read())) != Packet.PREAMBLE_A) {
         if (preamble == -1)
            break;
         //else
            //System.err.printf("Got something other than the preamble: %d\n", 
            //   preamble);
      }
      if (preamble == -1) // eof
         return false;
      //System.err.printf("Got the first preamble!\n");
      if ((preamble = (byte) this.is.read()) != Packet.PREAMBLE_B) 
         return false;
      //System.err.printf("Got the second preamble!\n");

      return true;
   }

   public Packet getPacket() throws IOException {
      int rlen;
      byte plen;
      byte recv_data[];

      // Get the preamble first
      if (!getPreambles()) 
         return null;

      // Get the length first and then the data
      plen = (byte) (this.is.read());
      if (plen == -1)
         return null;
      plen = (byte) (plen + Packet.HEADER_LEN);
      //System.out.printf("Packet len: %d\n", plen);

      recv_data = new byte[plen];
      recv_data[0] = (byte) (plen - Packet.HEADER_LEN);
      for (int i = 0; i < plen - 1; i += rlen) {
         rlen = this.is.read(recv_data, 1 + i, (plen - 1) - i);
         if (rlen == -1) {
            System.err.printf("ERROR: Reached EOF before reading the entire " +
                  "packet!\n");
            return null;
         }
      }

      return new Packet(recv_data);
   }

   // concatenate the preamble onto this packet and then
   // send all the data at once
   public void sendPacket(Packet thePacket) throws IOException {
      byte bPacket[] = thePacket.getBytes();
      byte withPreambles[] = new byte[bPacket.length + 2];
      withPreambles[0] = Packet.PREAMBLE_A;
      withPreambles[1] = Packet.PREAMBLE_B;

      System.out.printf("Sending packet: %s\n", thePacket.toString());

      System.arraycopy(bPacket, 0, withPreambles, 2, bPacket.length);
      try {
         os.write(withPreambles);
         os.flush();
      } catch (IOException e) {
         System.err.printf("Error: Failed to write to the port: %s\n",
               e.getMessage());
         closePort();
         System.exit(1);
      }
   }
}
