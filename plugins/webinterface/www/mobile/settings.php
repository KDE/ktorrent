<html>
<head>
<title>KTorrent WebInterface</title>
</head>
<body>
<table  width="100%">
  <tbody>
    <tr>
      <td align="center"><IMG src="ktorrentwebinterfacelogo.png" width="340" height="150" align="top" border="0"></td>
      <td><strong>ktorrent-><a href="interface.php">transfers</a></strong>->settings</td>
      <td><a href="settings.php" >refresh</a></td>
    </tr>
  </tbody>
</table>
<table  width="100%">
  <tbody>
<?php 
      		$globalinfo=globalInfo();
		echo "<FORM method=\"GET\">";
		echo "<tr>";
			echo "<td>Upload Speed (0 is no limit): </td>";
			echo "<td><INPUT type=\"text\" name=\"maximum_upload_rate\" value=\"{$globalinfo['max_upload_speed']}\"></td>";
		echo " </tr>";
		echo "<tr>";
			echo "<td>Download Speed (0 is no limit): </td>";
			echo "<td><INPUT type=\"text\" name=\"maximum_download_rate\" value=\"{$globalinfo['max_download_speed']}\"></td>";
		echo "</tr>";
		echo "<tr>";
			echo "<td>Maximum downloads (0 is no limit): </td>";
			echo "<td><INPUT type=\"text\" name=\"maximum_downloads\" value=\"{$globalinfo['max_downloads']}\"></td>";
		echo"</tr>";
		echo "<tr>";
			echo "<td>Maximum seeds (0 is no limit): </td>";
			echo "<td><INPUT type=\"text\" name=\"maximum_seeds\" value=\"{$globalinfo['max_seeds']}\"></td>";
		echo"</tr>";
		echo "<tr><td><INPUT type=\"submit\"></tr></td>";
		echo "</FORM>";
?>
</tbody>
</table>

</body>
</html>

