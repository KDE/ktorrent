<?php
	$globalinfo=globalInfo();
	$stats=downloadStatus(); 
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" 
>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
  <head>
    <style type="text/css" media="all">@import "stylen.css";

	.settingsInput
	{
		width:50px;
	}

    </style>
    <meta http-equiv="Content-Type" content="text/html"/>
    <link rel="icon" href="favicon.ico" type="image/x-icon"/>
    <link rel="shortcut icon" href="favicon.ico" type="image/x-icon"/>
    <title><?php echo '(D:'.$globalinfo['download_speed'].') (U:'.$globalinfo['upload_speed'].') KTorrent'; ?></title>
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
      <div id="top_bar">
        WebInterface KTorrent plugin
      </div>
      <div id="icon">
        <img src="icon.png" alt="KT Icon"/>
      </div>
      <div id="header">
        <div id="head-content">			
          <strong>KTorrent WebInterface</strong>
          <br />
          <font size="-1">BitTorrent client for KDE</font>
        </div>
		<div id="menu">
				<div style="padding-left:130px;">

					<dl>
							<dt><a href="interface.php" title="REFRESH">Refresh</a></b></dt>
							<dt><a href="login.html" title="LOGOUT">Logout</a></dt>
					</dl>
				</div>
			</div>
      </div>
      <div id="sidebar">
        <div id="sidebar-content">
			<!-- Control -->
			<div align="center" style="padding-bottom: 10px;">
			<table class="box" align="center" cellspacing="0" cellpadding="2">
                          <tr>
                            <td class="box-header">
                               Torrent control 
                            </td>
                          </tr>
                          <tr>
                            <td>
                          
                          <tr align="center">
                            <td>
                              <form action="interface.php" method="get">
                                <input type="submit" name="startall" value="Start All" class="buttons" />
                              </form>
                              <form action="interface.php" method="get">
                                <input type="submit" name="stopall" value="Stop All" class="buttons" />
                              </form>
                            </td>
                          </tr>
                          </td>
                        </table>
			</div>
          <!-- SETTINGS  -->
          <div align="center" style="padding-bottom: 10px;">
            <table cellspacing="0" cellpadding="2" class="box" style="width:250px;">
              <tbody>
                <tr>
                  <td class="box-header">
                     Settings 
                  </td>
                </tr>
                <tr>
                  <td>
                    <form action="interface.php" method="get">
                      <table class="box-settings" style="text-align:left;">
                        <tr>
                          <td>
                             Upload speed 
                          </td>
                          <td>
                            <input type="text" name="maximum_upload_rate" value="<?php echo $globalinfo['max_upload_speed']; ?>" class="settingsInput" />
                          </td>
                        </tr>
                        <tr>
                          <td>
                             Download speed 
                          </td>
                          <td>
                            <input type="text" name="maximum_download_rate" value="<?php echo $globalinfo['max_download_speed']; ?>" class="settingsInput" />
                          </td>
                        </tr>
                        <tr>
                          <td>
                             Maximum downloads 
                          </td>
                          <td>
                            <input type="text" name="maximum_downloads" value="<?php echo $globalinfo['max_downloads']; ?>" class="settingsInput" />
                          </td>
                        </tr>
                        <tr>
                          <td>
                             MaximumSeeds 
                          </td>
                          <td>
                            <input type="text" name="maximum_seeds" value="<?php echo $globalinfo['max_seeds']; ?>" class="settingsInput" />
                          </td>
                        </tr>
                        <tr>
                          <td>
                          </td>
                          <td>
                          </td>
                        </tr>
                        <tr>
                          <td colspan="2" style="text-align:right;">
                            <input type="submit" value="Submit settings" class="buttons"/>
                          </td>
                        </tr>
                        </form>
                      </table>
                      </td>
                      </tr>
                      </tbody>
                      </table>
            </div>
          <!-- Torrent -->
          <div align="center" style="padding-bottom: 10px;">
                        <table cellspacing="0" cellpadding="2" class="box">
                          <tbody>
                            <tr>
                              <td class="box-header">
                                 Load Torrents 
                              </td>
                            </tr>
                            <form action="interface.php" method="get">
                              <tr>
                                <td>
                                   Torrent URL: <input type="text" name="load_torrent" style="width:240px;" />
                                </td>
                              </tr>
                              <tr>
                                <td style="text-align:right;">
                                  <input type="submit" value="Load Torrent" class="buttons" />
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
                                   Local File:<input type="file" name="load_torrent" style="width:240px;">
                                </td>
                              </tr>
                              <tr>
                                <td style="text-align:right;">
                                  <input type="submit" name="Upload Torrent" value="Upload Torrent" class="buttons">
                                </td>
                              </tr>
                            </form>
                            <tr>
                              <td>
                                <hr/>
                              </td>
                            </tr>
                            <tr align="center">
                              <td>
                              </td>
                            </tr>
                          </tbody>
                        </table>                        
                      </div>
      
</div>
								  
   </div>
      <div id="content">
        <table>
          <tr>
            <td align="left" style="padding:2px;">
              <strong>Actions</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>File</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Status</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Downloaded</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Size</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Uploaded</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Down Speed</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Up Speed</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Peers</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Complete</strong>
            </td>
          </tr>
          <?php
				$stats=downloadStatus();
				$a = 0;
				foreach ($stats as $torrent) {
					echo "<tr>";
					
					$torrent_name = str_replace("'", "\'", $torrent['torrent_name']);
					
					if($torrent['total_bytes_to_download']!=0)
						$perc = round(100.0 - ($torrent['bytes_left_to_download'] / $torrent['total_bytes_to_download']) * 100.0, 2);
					else
						$perc = 0;
					echo "<td><a href=\"interface.php?stop=$a\" title=\"STOP\"><img src=\"/stop.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a>";
					echo "<a href=\"interface.php?start=$a\" title=\"START\"><img src=\"/start.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a>";
					echo "<a href=\"interface.php?remove=$a\" title=\"REMOVE\" onClick=\"return validate()\"><img src=\"/remove.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\"></a></td>";

					if(strlen($torrent['torrent_name'])>30)
						$display_name=substr($torrent['torrent_name'], 0, 30)." ...";
					else
						$display_name=$torrent['torrent_name'];
					
					if ($torrent['num_files']>1)
						echo "<td style=\"text-align:left;\" onmouseover=\"this.T_TITLE='$torrent_name';return escape('Download speed:<strong>{$torrent['download_rate']}</strong><br> Upload speed:<strong>{$torrent['upload_rate']}</strong></td>')\"><a href=\"details.php?torrent=$a\">$display_name</a></td>";
					else
						echo "<td style=\"text-align:left;\" onmouseover=\"this.T_TITLE='$torrent_name';return escape('Download speed:<strong>{$torrent['download_rate']}</strong><br> Upload speed:<strong>{$torrent['upload_rate']}</strong></td>')\">$display_name</td>";

					switch ($torrent['status']) {
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
					echo "<td style=\"text-align:right;\">{$torrent['bytes_downloaded']}</td>";
					echo "<td style=\"text-align:right; padding-left:8px;\">{$torrent['total_bytes']}</td>";
					echo "<td style=\"text-align:right; padding-left:8px;\">{$torrent['bytes_uploaded']}</td>";
					echo "<td style=\"text-align:right;\">{$torrent['download_rate']}</td>";
					echo "<td style=\"text-align:right;\">{$torrent['upload_rate']}</td>";
					echo "<td>{$torrent['num_peers']}</td>";
					echo "<td style=\"text-align:right;\">$perc %</td>";
					echo "</tr>";
				$a++;
				}
          ?>
        </table>
      </div>
      <div id="footer">
        <table width="100%" align="center">
          <tr>
            <td>
            </td>
            <td>
               &#169; 2006 WebInterface KTorrent plugin 
            </td>
            <td>
              <a href="login.html" title="LOGOUT">Logout</a> |  <a href="shutdown.php?quit=quit" title="Shutdown KTorrent">Shutdown KTorrent</a>
            </td>
          </tr>
        </table>
      </div>
 
    <script type="text/javascript" src="wz_tooltip.js"></script>
  </body>
</html>
