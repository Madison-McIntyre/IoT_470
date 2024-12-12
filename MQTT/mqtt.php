<?php
// Enable error reporting
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);


// Database connection details
$host = "localhost"; // Replace with your database host
$username = ""; // Replace with your database username
$password = ''; // Replace with your database password
$database = ""; // Replace with your database name

// Create connection
$conn = new mysqli($host, $username, $password, $database);

// Check connection
if ($conn->connect_error) {
    echo "Connection error.";
    die("Connection failed: " . $conn->connect_error);
}

$potValue = NULL;

foreach ($_REQUEST as $key => $value) {
    if ($key == "pot") {
        $potValue = $value;
    }
}

if (isset($potValue)) {
    $sql = "INSERT INTO `potentiometer_data`(`value`) VALUES ('" . $potValue . "')";
    if (!$conn->query($sql)) {
        echo "Error with SQL: " . $conn->error;
    }
}

// Query to get the newest value from the table
$sql = "SELECT `value` FROM `potentiometer_data` ORDER BY `id` DESC LIMIT 1";
$result = $conn->query($sql);

if ($result && $result->num_rows > 0) {
    $row = $result->fetch_assoc();
    echo "<p>Newest Potentiometer Value: " . htmlspecialchars($row['value']) . "</p>";
} else {
    echo "<p>No data available or query failed.</p>";
}

$conn->close();
?>

</body>
</html>

