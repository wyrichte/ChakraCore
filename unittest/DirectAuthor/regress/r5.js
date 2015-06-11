var TextHelper = {
       
       /**
       * Split long string into string array where each element fits within the line dimensions specified
       * by fontsize and linewidth parameters
       * @param {Object} text
       * @param {Object} fontsize
       * @param {Object} linewidth
       */
       LineLayout : function(text, fontsize, linewidth) {
              
              var retVal = new Array();
              
              var charactersPerLine = Math.round(Math.floor(linewidth/fontsize) * 2.3);
              
              var splitAt = charactersPerLine;
              var startAt = 0;
              
              startAt.|

              while (startAt < text.length - 1) {
                     
                     if (splitAt < text.length - 1) {
                           while (text.charAt(splitAt) != " ") {
                                  splitAt = splitAt - 1;
                           }
                           
                           
                           retVal.push(text.substring(startAt, splitAt));
                           startAt = splitAt + 1;
                           splitAt += charactersPerLine;
                     } else {
                           retVal.push(text.substring(startAt));
                           break;
                     }
              }
              
              return retVal;
              
       },     
       
       /**
       * Get number of characters pr line
       */
       CharactersPerLine : function(fontsize, linewidth) {
              return Math.round(Math.floor(linewidth/fontsize) * 2.4);
       }
       
};
 
