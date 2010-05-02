// encrypts the password and challenge fields using sha1
function encrypt_pwd()
{
	var usernameField = document.getElementById("username");
	var pwdField = document.getElementById("password");
	var challengeField = document.getElementById("challenge");
	var sha = hex_sha1(challengeField.value + pwdField.value);
	pwdField.value = '';
	challengeField.value = sha;
	return true;
}

// get the challenge field
function get_challenge()
{
	fetch_xml("login/challenge.xml",set_challenge,show_error)
}

function set_challenge(xmldoc)
{
	var challenge = document.getElementById("challenge");
	var c = xmldoc.getElementsByTagName("challenge");
	if (c[0])
		challenge.value = c[0].firstChild.data;
}

function show_error(msg)
{
	var p = document.getElementById("error_text");
	p.innerHTML = "Error: " + msg;
	p.style.display = "";
}