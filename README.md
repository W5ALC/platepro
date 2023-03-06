# platepro
Prepress preparation tool for PDF files.  

### Build and Install
Enter directory src  
`cd src`  
run  
`make -j4`

`make install`
Install manpage  
`make installman`  


### Features
* PDF v1.7 support  
* Decrypt encrypted PDFs  
* Join or Split PDFs  
* Scale to any paper size, with specified margin  
* Write Page numbers  
* Write text  
* Transform pages (rotate, flip, move)  
* Booklet format arrange  
* 2 or 4 pages per page (2-up, 4-up)  
* More readable output syntax for easy debugging  

### Usage
See manual page (PDF or man page) for detailed usage  

Scale to print in A4 size paper  
`platepro 'scaleto(a4)' input.pdf output.pdf`  

Add binding margin after scaling (? for odd pages, + for even pages)  
`platepro 'scaleto(a4) move(20){?} move(-20){+}' input.pdf output.pdf`  

Add page numbers  
`platepro 'number' input.pdf output.pdf`  

Booklet format  
`platepro 'book nup(2, paper=a4)' input.pdf output.pdf`  

adapted from 
 https://github.com/ksharindam/pdfcook
