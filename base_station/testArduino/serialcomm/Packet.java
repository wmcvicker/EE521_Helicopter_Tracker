package serialcomm;

import java.lang.Enum;
import java.lang.System;
import java.lang.Byte;
import java.lang.Short;

public class Packet {
   public enum PACKET_TYPE {
      INIT_T, GPS_DATA_T, SERVO_CTRL_T, HEADING_T, REQ_STATUS_T, STATUS_T, NEW_MISSION_T,
      MISSION_COMP_T, MISSION_ABORT_T, CORRUPT_T
   }; 
   public static final byte HEADER_LEN = 3;
   public static final byte PREAMBLE_A = (byte) 0xAA;
   public static final byte PREAMBLE_B = (byte) 0x55;

   public byte dlen;
   public byte dev_id;
   public PACKET_TYPE ptype;
   public byte data[];

   public Packet(byte len, byte did, byte type, byte d[]) {
      this.dlen = len;
      this.dev_id = did;
      this.ptype = PACKET_TYPE.values()[type];

      if (len > 0) {
         data = new byte[len];
         System.arraycopy(d, 0, this.data, 0, len);
      }
   }

   public Packet(byte len, byte did, PACKET_TYPE type, byte d[]) {
      this.dlen = len;
      this.dev_id = did;
      this.ptype = type;

      if (len > 0) {
         data = new byte[len];
         System.arraycopy(d, 0, this.data, 0, len);
      }
   }

   public Packet(byte buffer[]) {

      if (buffer.length < 3)
         return;
      
      dlen = buffer[0];
      // Make sure the packet is not corrupt
      if (buffer.length < (dlen + HEADER_LEN)) {
         ptype = PACKET_TYPE.CORRUPT_T;
         return;
      }
      
      dev_id = buffer[1];
      if (buffer[2] >= (PACKET_TYPE.values()).length) {
         ptype = PACKET_TYPE.CORRUPT_T;
         return;
      } else
         ptype = PACKET_TYPE.values()[buffer[2]];

      if (dlen > 0) {
         data = new byte[dlen];
         System.arraycopy(buffer, HEADER_LEN, data, 0, dlen);
      }
   }

   // Copy the data into a byte array and pre-pend the peamble
   // to the packet
   public byte[] getBytes() {
      byte buffer[] = new byte[HEADER_LEN + dlen];

      buffer[0] = dlen;
      buffer[1] = dev_id;
      buffer[2] = (byte) (ptype.ordinal());
      for (int i = 0; i < dlen; i++) 
         buffer[HEADER_LEN + i] = data[i];
      
      return buffer;
   }

   @Override public String toString() {
      String temp, pdata;

      temp = Byte.toString(dlen) + ", " + Byte.toString(dev_id) + ", " +
         ptype.toString() + ", ";

      if (data == null)
         return temp;
      else if (ptype == PACKET_TYPE.GPS_DATA_T) {
         double lat_deg, lon_deg, lat_milli_min, lon_milli_min;
         double lat_min, lon_min;
         byte quad;

         lat_deg = (new Short((short) ((data[1] << 8) | (data[0] & 0x00ff)))).doubleValue();
         lon_deg = (new Short((short) ((data[6] << 8) | (data[5] & 0x00ff)))).doubleValue();
         lat_min = (new Byte(data[2])).doubleValue();
         lon_min = (new Byte(data[7])).doubleValue();
         lat_milli_min = (new Short((short) ((data[4] << 8) | (data[3] & 0x00ff)))).doubleValue();
         lon_milli_min = (new Short((short) ((data[9] << 8) | (data[8] & 0x00ff)))).doubleValue();
         quad = data[10];

         // Got GPS coordinates
         double h_lat = lat_deg + lat_min*0.01 + lat_milli_min*0.000001;
         double h_lon = lon_deg + lon_min*0.01 + lon_milli_min*0.000001;
         
         return temp + Double.toString(h_lat) + ", " + 
            Double.toString(h_lon) + ", " + Byte.toString(quad);
      } else
         pdata = new String(data);

      return temp + pdata;
   }
}
