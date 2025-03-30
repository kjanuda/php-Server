You said:
<?php
$hostname = "localhost";
$username = "root";
$password = "";
$database = "sensor_db"; 

$conn = mysqli_connect($hostname, $username, $password, $database);
if (!$conn) { 
    die("Connection failed: " . mysqli_connect_error());
}
echo "Database Connection is Ok<br>";

// Validate POST data
$t = isset($_POST["temperature"]) ? $_POST["temperature"] : null;
$h = isset($_POST["humidity"]) ? $_POST["humidity"] : null;
$a = isset($_POST["airQuality"]) ? $_POST["airQuality"] : null;

// Check if any value is missing
if ($t === null || $h === null || $a === null) {
    die("ERROR: Missing data. Ensure all parameters are sent.");
}

// Prevent SQL injection by escaping values
$t = mysqli_real_escape_string($conn, $t);
$h = mysqli_real_escape_string($conn, $h);
$a = mysqli_real_escape_string($conn, $a);

// Correct SQL query with single quotes for string values
$sql = "INSERT INTO dht11 (temperature, humidity, airQuality) VALUES ('$t', '$h', '$a')";

if (mysqli_query($conn, $sql)) {
    echo "New Record Successfully Inserted";
} else {
    echo "ERROR: " . mysqli_error($conn);
}

// Close the database connection
mysqli_close($conn);
?>