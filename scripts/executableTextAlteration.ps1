# Define the path to the executable file
$filename = "./game.exe"

# Define the custom message to replace the original MZ part
$customMessage = "Even ninjas cannot let this run on DOS.   "

# Convert the custom message to a byte array
$customBytes = [System.Text.Encoding]::ASCII.GetBytes($customMessage)

# Open the executable file in binary mode and modify the MZ part
$fileStream = [System.IO.File]::OpenWrite($filename)
$fileStream.Seek(78, 'Begin')
$fileStream.Write($customBytes, 0, $customBytes.Length)
$fileStream.Close()
