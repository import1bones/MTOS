Manager Memory Unit
Most modern systems use page to virtual memory,if I set page size is 4kib(default),I have a virtual memory address, how could I know physical address ?
A figure can help me, virtual memory address(32 bits system) is consist of three part , Page Directory Index, Page Table Index ,Page Offset.
*	|31	<-	 10bits   -> 22|21 <- 10bits -> 12|11 <-    12bits  -> 0|
*	+----------------------+------------------+---------------------+
*	| Page Directory Index | Page Table Index |		Page Offset		|
*	+----------------------+------------------+---------------------+	
*				|			   |						  |
*	+-----------+			   |						  |	
*	|						   |						  |		+-----------+
*	|	  +------------+	   |	+-----------+		  +---> |  	 ans 	|
*	|     | 		   |	   |	|			|				+-----------+
*	| 	  | 		   |	   |	+-----------+				|			|
*	|	  | 		   |	   +--> |	 PTX    |-------+		|			|
*	|	  +------------+			+-----------+		|		|			|
*	+---> |		PDX	   |-----+		|			|		|		|			|
*		  +------------+	 |		|			|		|		|			|
*		  |			   |	 |		|			|		|		|			|
*		  | 		   |	 |		|			|		|		|			|
*		  +------------+	 |		+-----------+		|		+-----------+
*							 +------> Page table		+-------> Page Frame
