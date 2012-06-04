/** This application interfaces with an MSP430 that is wirelessly connected to 
 * a helicopter and attempts to track the helicopter using a GPS.
 *
 * @author William McVicker
 */

import javax.swing.*;

import java.awt.*;
import java.awt.event.*;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.IOException;
import java.lang.InterruptedException;

import basestation.*;
import basestation.serialcomm.*;

public class BS_GUI extends JPanel implements ActionListener {
   protected static final String newMission_AC = "newmission";
   protected static final String mapsBaseURL =
      "http://maps.googleapis.com/maps/api/staticmap?";
   //   protected InputStream is;
   protected BaseStation thisStation;
   Thread missionThread, updateMapThread;

   // Lat & Lon 
   private JTextField latField;
   private JTextField lonField;
   private JButton newMission_btn;
   private JLabel mapLabel;

   public BS_GUI(String portName) throws IOException {
      try {
         thisStation = new BaseStation(portName);
      } catch (Exception e) {
         System.err.printf("Failed to setup base station\n");
         System.exit(1);
      }

      BufferedImage mapImage;

      setLayout(new BorderLayout());

      JPanel leftPane = new JPanel();
      setupLeftPane(leftPane);
      leftPane.setBorder(BorderFactory.createCompoundBorder(
               BorderFactory.createTitledBorder("Mission Controls"),
               BorderFactory.createEmptyBorder(5,5,5,5)));

      
      mapImage = getMap(thisStation.getHelicopter_lat(), 
            thisStation.getHelicopter_lon());
      mapLabel = new JLabel(new ImageIcon(mapImage));

      JPanel rightPane = new JPanel(new GridLayout(1,0));
      rightPane.add(mapLabel);
      rightPane.setBorder(BorderFactory.createCompoundBorder(
               BorderFactory.createTitledBorder("Google Maps View"),
               BorderFactory.createEmptyBorder(5,5,5,5)));


      add(rightPane, BorderLayout.LINE_END);
      add(leftPane, BorderLayout.LINE_START);
   }

   private void setupLeftPane(Container container) {
      GridBagLayout gridbag = new GridBagLayout();
      GridBagConstraints c = new GridBagConstraints();
      container.setLayout(gridbag);

      latField = new JTextField(15);
      lonField = new JTextField(15);

      JLabel latLabel = new JLabel("Latitude ");
      latLabel.setLabelFor(latField);
      JLabel lonLabel = new JLabel("Longitude ");
      lonLabel.setLabelFor(lonField);

      JLabel labels[] = {latLabel, lonLabel};
      JTextField textFields[] = {latField, lonField};
      addToPane(labels, textFields, gridbag, container);

      // Create a button to start new missions
      newMission_btn = new JButton("New Mission");
      newMission_btn.setVerticalTextPosition(AbstractButton.CENTER);
      newMission_btn.setMnemonic(KeyEvent.VK_M);
      newMission_btn.setActionCommand(newMission_AC);
      newMission_btn.addActionListener(this);
      c.anchor = GridBagConstraints.CENTER;
      c.gridwidth = GridBagConstraints.REMAINDER;
      c.fill = GridBagConstraints.NONE;
      c.weightx = 0.0;
      container.add(newMission_btn, c);
  
   }

   private void addToPane(JLabel[] labels, JTextField[] textFields,
         GridBagLayout gridbag, Container container) {
      GridBagConstraints c = new GridBagConstraints();
      c.anchor = GridBagConstraints.EAST;

      int numLabels = labels.length;
      for (int i = 0; i < numLabels; i++) {
         c.gridwidth = GridBagConstraints.RELATIVE; //next-to-last
         c.fill = GridBagConstraints.NONE;      //reset to default
         c.weightx = 0.0;                       //reset to default
         container.add(labels[i], c);

         c.gridwidth = GridBagConstraints.REMAINDER;     //end row
         c.fill = GridBagConstraints.HORIZONTAL;
         c.weightx = 1.0;
         container.add(textFields[i], c);
      }
   }

   public BufferedImage getMap(double lat, double lon) throws IOException {
      BufferedImage mapImage = null;

      try {
         mapImage = ImageIO.read(new URL(mapsBaseURL + "center=" +
                  Double.toString(lat) + "," + Double.toString(lon) +
                  "&zoom=18&size=512x512&maptype=satellite&" +
                  "markers=color:blue%7Clabel:H%7C" + Double.toString(lat) + 
                  "," + Double.toString(lon) + "&sensor=true"));
      } catch (MalformedURLException e) {
         System.err.printf("Failed to retrieve the Google Static Map\n");
      }

      return mapImage;
   }

   public BufferedImage getMap(double curr_lat, double curr_lon, 
         double dest_lat, double dest_lon) throws IOException {
      BufferedImage mapImage = null;

      try {
         mapImage = ImageIO.read(new URL(mapsBaseURL + "center=" +
                  Double.toString(/*(dest_lat + curr_lat)/2*/curr_lat) + "," + 
                  Double.toString(/*(dest_lon + curr_lon)/2*/curr_lon) +
                  "&zoom=19&size=512x512&maptype=satellite&" +
                  "markers=color:blue%7Clabel:H%7C" + 
                  Double.toString(curr_lat) + "," + Double.toString(curr_lon) +
                  /*"&markers=color:red%7Clabel:D%7C" + 
                  Double.toString(dest_lat) + "," + Double.toString(dest_lon) +*/
                  "&sensor=true"));
      } catch (MalformedURLException e) {
         System.err.printf("Failed to retrieve the Google Static Map\n");
      }

      return mapImage;
   }

   public void actionPerformed(ActionEvent e) {

      if (newMission_AC.equals(e.getActionCommand())) {
         double lat, lon;

         newMission_btn.setEnabled(false);
         latField.setEnabled(false);
         lonField.setEnabled(false);
         lat = Double.parseDouble(latField.getText());
         lon = Double.parseDouble(lonField.getText());
         try {
            updateMapThread = new Thread(new UpdateMap(lat, lon));
         } catch (IOException ex) {}
         updateMapThread.start();
         missionThread = thisStation.startMission(lat, lon, updateMapThread);
      }

   }

   private class UpdateMap implements Runnable {
      protected double d_lat, d_lon;
      protected double h_lat, h_lon;
      BufferedImage mapImage;

      public UpdateMap(double dest_lat, double dest_lon) throws IOException {
         d_lat = dest_lat;
         d_lon = dest_lon;

         updateMap();
      }

      private void updateMap() throws IOException {
         h_lat = thisStation.getHelicopter_lat();
         h_lon = thisStation.getHelicopter_lon();

         mapImage = getMap(h_lat, h_lon, d_lat, d_lon);
         mapLabel.setIcon(new ImageIcon(mapImage));

         System.out.printf("DEBUG: updated map to %f, %f\n", h_lat, h_lon);
      }

      public void run() {
         boolean getGPS = false;

         while (true) {
            try {
               Thread.sleep(5000);
            } catch (InterruptedException e) {
               getGPS = true;
            }
            if (!getGPS)
               continue;
            else
               getGPS = false;

            // retrieve the new GPS coordinates and update the map 
            try {
               updateMap();
            } catch (IOException e) {
               System.err.printf("Failed to update the map\n");
            }
         }

         /*latField.setText("");
         lonField.setText("");
         latField.setEnabled(true);
         lonField.setEnabled(true);
         newMission_btn.setEnabled(true);*/
      }
   }

   private static void createAndShowGUI(String portName) throws IOException {

      // Create and setup the window
      JFrame frame = new JFrame("AutoCopter Base Station");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

      BS_GUI newContentPane = new BS_GUI(portName);
      newContentPane.setOpaque(true);
      frame.setContentPane(newContentPane);

      frame.pack();
      frame.setVisible(true);
   }


   public static void main(String[] args) throws IOException {
      final String portName = args[0];

      javax.swing.SwingUtilities.invokeLater(new Runnable() {
         public void run() {
            try {
               createAndShowGUI(portName);
            } catch (Exception e) {}
         }
      });
   }
}
