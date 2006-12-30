<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
    <head>
        <style type="text/css" media="all">@import "stylen.css";</style>
	<?php
		$globalinfo=globalInfo();
        	echo "<title>(D:$globalinfo[download_speed]) (U:$globalinfo[upload_speed]) KTorrent</title>";
	?>
        <meta http-equiv="Content-Type" content="text/html"/>
        <link rel="icon" href="favicon.ico" type="image/x-icon"/>
        <link rel="shortcut icon" href="favicon.ico" type="image/x-icon"/>
	
	<script language="JavaScript">

	function validate()
	{
		msg = "Are you absolutely sure that you want remove this torrent?";
    		return confirm(msg);
	}

	</script>
    </head>
    <body>
        <div id="page-container">
            <div id="top_bar"><a href="interface.php" title="REFRESH"><strong>refresh</strong></a></b></div>
         
        <div id="icon">
            <img src="icon.png" alt="KT Icon"/>
        </div>
        
        <div id="header">
            <div id="head-content">
                <strong>KTorrent WebInterface</strong><br />
                <font size="-1">BitTorrent client for KDE</font>
            </div>
            
            
        </div>
        <div id="sidebar">
            <div id="sidebar-content">
                <!-- SETTINGS  -->
                <div align="center" style="padding-bottom: 10px;">
                    <table cellspacing="0" cellpadding="1" class="box">
                        <tbody>
                            <tr>
                                <td class="box-header">Settings</td>
                            </tr>
                            <tr>
                                <td>
				    <table class="box-settings">
                                        <?php
					echo "<form action=\"interface.php\" method=\"get\">";
					echo "<tr><td><input type=\"text\" name=\"maximum_upload_rate\" value=\"$globalinfo[max_upload_speed]\" /></td><td> UploadSpeed</td></tr>";
					echo "<tr><td><input type=\"text\" name=\"maximum_download_rate\" value=\"$globalinfo[max_download_speed]\" /></td><td> DownloadSpeed</td></tr>";
					echo "<tr><td><input type=\"text\" name=\"maximum_downloads\" value=\"$globalinfo[max_downloads]\" /></td><td> MaximumDownload</td></tr>";
					echo "<tr><td><input type=\"text\" name=\"maximum_seeds\" value=\"$globalinfo[max_seeds]\" /></td><td> MaximumSeeds</td></tr>";
					echo "<tr align=\"center\"><td><input type=\"submit\" value=\"Submit Settings\"/></td></tr>";
					echo "</form>";
					?>
                                    </table>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
                
                <!-- Torrent -->
                <div align="center" style="padding-bottom: 10px;">
                    <table cellspacing="1" cellpadding="1" class="box">
                        <tbody>
                            <tr>
                                <td class="box-header">Load Torrents</td>
                                
                            </tr>
                            
                            <form action="interface.php" method="get">
                                <tr>
                                    <td><input type="text" name="load_torrent" />
                                        <input type="submit" value="Load Torrent" />
                                    </td>
                                </tr>
                            </form>
                            <tr>
                                <td>
                                    <hr/>
                                </td>
                            </tr>
                            <form method="post" enctype="multipart/form-data" action="interface.php">
                                <tr>
                                    <td>
                                        Local File:<input type="file" name="load_torrent"></td>
                                    </tr>
                                    <tr>
                                        <td>
                                            <input type="submit" name="Upload Torrent" value="Upload Torrent"></td>
                                        </tr>
                                    </form>
                                    <tr>
                                        <td>
                                            <hr/>
                                        </td>
                                    </tr>
                                    
                                    <tr align="center">
                                        <td><form action="interface.php" method="get">
                                                <input type="submit" name="stopall" value="Stop All" />
                                            </td>
                                        </tr>
                                    </form>
                                    <form action="interface.php" method="get">
                                        <tr align="center">
                                            <td>
                                                <input type="submit" name="startall" value="Start All" />
                                            </td>
                                        </tr>
                                    </form>
                                    
                                </tbody>
                            </table>
                        </div>
                        
                    </div>
                </div>
                
                <div id="content">
                    <table>
                        <tr>
                            <td align="left">
                                <strong>Actions</strong>
                            </td>
                            <td align="left">
                                <strong>File</strong>
                            </td>
                            <td align="left">
                                <strong>Status</strong>
                            </td>
                            <td align="left">
                                <strong>Downloaded</strong>
                            </td>
                            <td align="left">
                                <strong>Size</strong>
                            </td>
                            <td align="left">
                                <strong>Uploaded</strong>
                            </td>
                            <td align="left">
                                <strong>Down Speed</strong>
                            </td>
                            <td align="left">
                                <strong>Up Speed</strong>
                            </td>
                            <td align="left">
                                <strong>Peers</strong>
                            </td>
                            <td align="left">
                                <strong>Complete</strong>
                            </td>
                        </tr>

			<?php
				$stats=downloadStatus();
				$a = 0;
				foreach ($stats as $torrent) {
					echo "<tr>";
					if($torrent[total_bytes_to_download]!=0)
						$perc = round(100.0 - ($torrent[bytes_left_to_download] / $torrent[total_bytes_to_download]) * 100.0, 2);
					else
						$perc = 0;
					echo "<td><a href=\"interface.php?stop=$a\" title=\"STOP\"><img src=\"/stop.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a>";
					echo "<a href=\"interface.php?start=$a\" title=\"START\"><img src=\"/start.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a>";
					echo "<a href=\"interface.php?remove=$a\" title=\"REMOVE\" onClick=\"return validate()\"><img src=\"/remove.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a></td>";
					echo "<td onmouseover=\"this.T_TITLE='$torrent[torrent_name]';return escape('Download speed:<strong>$torrent[download_rate]</strong><br> Upload speed:<strong>$torrent[upload_rate]</strong></td>')\">";
					if(strlen($torrent[torrent_name])>20)
						echo substr($torrent[torrent_name], 0, 20) . " ...</td>";
					else
						echo "$torrent[torrent_name]</td>";
					switch ($torrent[status]) {
						case 0:
   							echo "<td>Not Started</td>";
							break;
						case 1:
  							echo "<td>Seeding Complete</td>";
   							break;
						case 2:
							echo "<td>Download Complete</td>";
							break;
						case 3:
							echo "<td>Seeding</td>";
							break;
						case 4:
							echo "<td>Downloading</td>";
							break;
						case 5:
							echo "<td>Stalled</td>";
							break;
						case 6:
							echo "<td>Stopped</td>";
							break;
						case 7:
							echo "<td>Allocating Diskspace</td>";
							break;
						case 8:
							echo "<td>Error</td>";
							break;
						case 9:
							echo "<td>Queued</td>";
							break;
						case 10:
							echo "<td>Checking Data</td>";
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
					echo "<td>$perc %</td>";
					echo "</tr>";
				$a++;
				}
			?>
			
		</table>
		</div>

        	<div id="footer">
			<table width="100%" align="center">
			<tr>
				<td></td>
				<td>&#169; 2006 webinterface KTorrent plugin</td>
				<td><a href="login.html" title="LOGOUT">logout</a></td>
			</tr>
			</table>
		</div>

	</div>
	<script type="text/javascript" src="wz_tooltip.js"></script>
	</body>
</html>
