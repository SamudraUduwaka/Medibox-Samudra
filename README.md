<h2>MediBox: Smart Medicine Storage device</h2>

<h3>Project Overview</h3>

-The MediBox project offers an innovative approach to medicine storage, ensuring secure management and continuous monitoring of environmental conditions like temperature and humidity. The system integrates a variety of hardware components and utilizes a Node-RED dashboard for real-time data visualization and control.

<h3>Components and Features</h3>

<h4>ESP32 Module</h4>
<ul>
  <li>Role: Serves as the primary controller.</li>
  <li>Function: Manages data from sensors and handles user inputs.</li>
</ul>

<h4>DHT22 Sensor</h4>
<ul>
  <li>Role: Environmental monitoring.</li>
  <li>Function: Measures and reports temperature and humidity inside the MediBox.</li>
</ul>

<h4>OLED Display</h4>
<ul>
  <li>Role: User interface.</li>
  <li>Function: Displays vital information including current time, alarms, and environmental readings.</li>
</ul>

<h4>Push Buttons</h4>
<ul>
  <li>Role: User interaction.</li>
  <li>Function: Allow users to set time, configure alarms, and navigate through the system menu.</li>
</ul>

<h4>Node-RED Dashboard</h4>
<ul>
  <li>Purpose: Facilitates data visualization and control.</li>
  <li>Functionality: Provides a user-friendly interface to monitor environmental conditions and manage the MediBox settings remotely.</li>
</ul>

<h3>Simulation Setup Diagram</h3>

<img src="Simulation Setup.png"/>

<h3>Functionality Overview</h3>

<h4>Time Management</h4>
<ul>
  <li>Set Time: Users can configure the current time using the push buttons for accurate timekeeping within the MediBox.</li>
</ul>

<h4>Alarm Features</h4>
<ul>
  <li>Set Alarms:
Users can set up to three alarms for medicine reminders.
Alarm times can be adjusted using the push buttons, ensuring timely notifications.</li>
</ul>

<h4>Environmental Monitoring</h4>
<ul>
  <li>Temperature and Humidity Monitoring:
The DHT22 sensor continuously tracks the temperature and humidity levels inside the MediBox.
If readings fall outside the specified range, alerts are displayed to notify the user of potential issues.</li>
</ul>

<h4>Dashboard Integration</h4>
<ul>
  <li>Node-RED Dashboard:
Data from the MediBox, including temperature and humidity readings, are transmitted to the Node-RED dashboard via the MQTT protocol.
Users can visualize this data in real-time and make adjustments to the light conditions within the MediBox as needed through the dashboard interface.</li>
</ul>

<h4>Node-red Dashboard</h4>
<img src="Node-red Dashboard.png"/>
