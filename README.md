# Twitch API Windows Library

## WARNING
I started working on this library because i needed to read messages from a Twitch chat and then
i wrote a complete library but, this code lack a lot of features and functionalities,
everything work at the bare minimum, so **USE IT AT YOUR OWN RISK!!**

## HOW TO BUILD THE LIBRARY
It's simple, just run build_DLL_RELEASE.bat for the release version, or
run build_DLL_DEBUG.bat for the debug version.

```
build_DLL_RELEASE.bat
```

## HOW TO BUILD THE EXAMPLES
Even more simple, just run build_examples.bat for the release version, or
run build_examples_debug.bat for the debug version.

```
build_examples.bat
```

These two batches will build the dll and all the examples.
If you want to just build the single example run build_RELEASE.bat/build_DEBUG.bat 
with the example file you want to build as an argument.

```
build_RELEASE.bat src\api_example.c
```

## HOW TO USE

### Step 1
Get your client id and oauth token, if you don't know what i'm talking about 
go here https://dev.twitch.tv/docs/api/get-started/ and read the docs.

Init the FTWLUserCredentials struct, you can have multiple of them,
in my case i use more than one when i have more than one oauth tokens with different scopes.
```
FTWLUserCredentials api_creds_1 = {
	.client_id = L"sakjhdlkahjslkdjlakjdlkajslk",
	.oauth2_token = L"diuoasoi12093idjsaloijda",
};
```

### Step 2
Connect to the Twitch API server.
```
TWLConnect();
```

### Step 3
Set the current user, the first call to this function after the successfull
connection to the server is **mandatory**!!

`TWLSetCurrentUser(&api_creds_1);`

From here every calls to the API will use the **api_creds_1** user_id and token.

### Step 4
Call your API function, for example here we are going to get some twitch user information,
in this example we are going to use the "login name" to gather this information.
```
char *user_info_data = NULL;
TWLGetUsers(NULL, "ZioYuri78", &user_info_data);
```

An important concept of this library is that **almost every TWL function that get a buffer
as a parameter will allocate the memory for you**, in this case the call to TWLGetUsers
will allocate the memory block and will assign it to the user_info_data pointer.

This library also as few parsing functions like TWLPUserInfo, all the TWLP function
receive the pointer allocated with the corresponding TWL function, fill a struct and
**deallocate the memory for you**.

If instead you want to use your own parsing funtion you **must have to free the memory**
with the TWLFree function once you no longer need the data. 

### Step 4a
Use a TWLP function and the memory is freed for you.
```
FTWLUserInfo user_info = {};
FTWLPUserInfo(user_info_data, &user_info);
```

### Step 4b
Do your own things with the data and then manually free the memory.
```
/* 
 * some code here that do stuff with user_info_data
 */

TWLFree(user_info_data);
```

### Step 5
Shutdown the connection to the Twitch API server.
```
TWLShutDown();
```

## Conclusion
That's all, for more you can explore the three example:

**api_example.c**

**eventsub_example.c**

**multiple_user_example.c**

YB

    

