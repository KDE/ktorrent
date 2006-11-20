
<html>
<head>
<title>KTorrent WebInterface</title>
</head>
<body>
<table  width="100%">
  <tbody>
    <tr>
      <td>&nbsp;</td>
      <td align="center"><IMG src="ktorrentwebinterfacelogo.png" width="340" height="150" align="top" border="0"></td>
      <td>&nbsp;</td>
    </tr>
    <tr>
      <td>
      <table width="100%">

<?php 
      		$globalinfo=globalInfo();
		echo "<tr><td><strong> Settings </strong></tr></td>";
		echo "<FORM method=\"GET\">";
		echo "<tr><td><INPUT type=\"text\" name=\"maximum_upload_rate\" value=\"$globalinfo[max_upload_speed]\">UploadSpeed</tr></td>";
		echo "<tr><td><INPUT type=\"text\" name=\"maximum_download_rate\" value=\"$globalinfo[max_download_speed]\">DownloadSpeed</tr></td>";
		echo "<tr><td><INPUT type=\"text\" name=\"maximum_downloads\" value=\"$globalinfo[max_downloads]\">MaximumDownload</tr></td>";
		echo "<tr><td><INPUT type=\"submit\"></tr></td>";
		echo "</FORM>";
?>
		<tr><td><FORM method="GET">
		<INPUT type="text" name="load_torrent">
		<INPUT type="submit" name="Load torrent" value="Load torrent"></tr>
		</FORM></tr></td>

		<tr><td><FORM method="GET">
		<INPUT type="submit" name="stopall" value="stopall">
		</FORM></tr></td>
		<tr><td><FORM method="GET">
		<INPUT type="submit" name="startall" value="startall">
		</FORM></tr></td>
			</table>

      </td>
      <td>
		<table width="100%">
			<tr>
				<td align="left">Actions</td>
				<td align="left">File</td>
				<td align="left">Status</td>
				<td align="left">Downloaded</td>
				<td align="left">Size</td>
				<td align="left" >Uploaded</td>
				<td align="left">Down Speed</td>
				<td align="left">Up Speed</td>
				<td align="left">Peers</td>
				<td align="left">% Complete</td>
			</tr>
			
				<?php
				$stats=downloadStatus();
				$a = 0;
				foreach ($stats as $torrent) {
				$perc=$torrent[bytes_downloaded]*100/$torrent[total_bytes];
				echo "<tr>";
					echo "<td><a href=\"interface.php?stop=$a\" title=\"STOP\"><img src=\"/stop.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\"></a>";
					echo "<a href=\"interface.php?start=$a\" title=\"START\"><img src=\"/start.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\"></a>";
					echo "<a href=\"interface.php?remove=$a\" title=\"REMOVE\"><img src=\"/remove.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\"></a></td>";
					echo "<td>$torrent[torrent_name]</td>";
					switch ($torrent[status]) {
						case 0:
   							echo "<td>NOT_STARTED</td>";
							break;
						case 1:
  							echo "<td>SEEDING_COMPLETE</td>";
   							break;
						case 2:
							echo "<td>DOWNLOAD_COMPLETE</td>";
							break;
						case 3:
							echo "<td>SEEDING</td>";
							break;
						case 4:
							echo "<td>DOWNLOADING</td>";
							break;
						case 5:
							echo "<td>STALLED</td>";
							break;
						case 6:
							echo "<td>STOPPED</td>";
							break;
						case 7:
							echo "<td>ALLOCATING_DISKSPACE</td>";
							break;
						case 8:
							echo "<td>ERROR</td>";
							break;
						case 9:
							echo "<td>QUEUED</td>";
							break;
						case 10:
							echo "<td>CHECKING_DATA</td>";
							break;
						default:
							echo "<td>Not supported Status</td>";
					}
					echo "<td>$torrent[bytes_downloaded]</td>";
					echo "<td>$torrent[total_bytes]</td>";
					echo "<td>$torrent[bytes_uploaded]</td>";
					echo "<td>$torrent[download_rate]</td>";
					echo "<td>$torrent[upload_rate]</td>";
					echo "<td>$torrent[num_peers]</td>";
					echo "<td>$perc</td>";				
				echo"</tr>";
				$a=$a+1;
				}
			?>
			
		</table>
	</td>
      
    </tr>
  </tbody>
</table>


</body>
</html>

