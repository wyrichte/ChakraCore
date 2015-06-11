Collection of small Ajax/html pages that demo the Eze Dom capabilities.
The tests can be run manually with IE8 ( Eze Jscript library substituted for jscript.dll)


Description:

1. Allert5times.html
Description: Repeated call into the dom windows.alert() function passing a string. Also it calls in document.write().

Instructions: Hit 5 times ok :-)


2. DialogBoxConfirm.html
Description: Button event jscript function that calls into window.confirm. It displays a dialog box window and then computes a factorial. Events, back and forth calls into DOM. 
Instructions: Hit 'Start' on the dialog box.

3. OpenCloseWindow.html
Description: A new window is opened and closed using two button events. Yes we can open a new browser window from Eze. I will try to run SS in it. 

Instructions: hit Open /Close 


4.  FrameParent.html, Frame_a.html, Frame_b.html
Description: A multiframe scenario with a cross frame object. The parent frame defines a ccomplex number object. The two child Iframes read the object via widow.parent and display the real/imaginary components of the complex number. The scenario is important because we have three different engines and contexts here sharing  an object via DOM

Instructions: start FrameParent.html 


5 . OnLoadWriteInnerHtml.html
Description: The page defines a text box and a paragraph that has an inner html tagged by an ‘id’ . The original value of the inner html is preset. There is an Onload event that changes the inner html. Then a button event can change the inner html value with the textbox content. It is a combination of event scenario, read/writes/ invokes between the jscript engine and various DOM objects. 

Instructions: enter text in the box, press"ChangeText"


6. ImageRolover.html, ImageRoloverHelp.html, Ez1.jpg, Ez2.png.
Description: Classic image rollover scenario. On a first look no Jscript code. Actually the mouse events are done via anonymous Jscript functions.  Hover the mouse over the image, and click on image. 

Instructions: start ImageRolover.html , move the mouse over/out of the picture, click on the picture, find out what is Eze about.


7. LoanCalc.html. 
Description: Loan calculator, forms, events, object arithmetic.  (Flanagan example).

Instructions: Check your mortgage refi data


8. SearchYahoPipe.html
Description: Search the keyword 'Eze' on Yahoo pipe web service,  retain and display top 100 results. It is the first scenario when a server is involved. The name of the Jscript callback function that processes the result  is part of the url src string for  the web server. 

Instructions: start hit ok for the number of searches


9. BingEze.html
Description: A search application that takes the user input from a text box and displays the top search results in a result.
It demoes code injection, dynamic change of the source of a script element, events, text boxes. 
When the user hits 'Search' button first time, the event function injects a script element into the document.  The script source is set to yahoo search pipe passing the search params and the call back function. 
When the search is complete, the callback function processes the search result object and display the results  in  the output text box.

Instructions: type a word in the box, then hit 'Search' button. Sorry I do not know how to make it work with 'Return'


10. ActiveXrunFlash.html
Description: Instantiates the flash ActiveX and shows the version. 

Instructions: Hit ok on the alert boxes. 

11. ActiveXrunFlashVideo.html
Description: Instantiates the flash ActiveX and shows the version. Then injects the plugin tag for a movie and runs the movie

Instructions: enjoy the movie


12. ActiveXrunWSH.html
Description: Instantiates wsh ActiveX object and then runs it with a jacript file in the cmd line. 

Instructions: Hit yes and ok to all dialog boxes.

13. XMLHttpRequest. 
Description: It is the core scenario for Ajax: create a new XMLHttpRequest, open the request( a web service is used), specify the handler for onreadystate event, process the result in the handler. 

Instructions. Just open the html file. The test doesn't need a server. 


On testing:

1.  Forms.html
Description: A demo of various components of a form and events hooks. (Flanagan example).
Mostly it works but the full correct functionality is not validated yet.

Instructions: play with the forms, observe the input events window


Not working yet:

1. WindowBounce.html
Description: Opens a window that slides and bounces.


 