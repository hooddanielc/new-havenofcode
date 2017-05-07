= consumer-header
h1.text-center Your Files
= upload-file-form files=model
each model as |file|
  p
    b Name: 
    | {{file.name}} 
    b Aws Key: 
    | {{file.awsKey}} 
    b Aws Region: 
    | {{file.awsRegion}} 
    b Bytes: 
    | {{file.bytes}} 
    b Created At: 
    | {{file.createdAt}} 
    b Created By: 
    | {{file.createdBy.email}} 
    b ID: 
    | {{file.id}} 
    b Status: 
    | {{file.status}} 
    b Updated At: 
    | {{file.updatedAt}} 
    b Url: 
    | {{file.url}}
    if file.url
      img src=file.url style="max-width: 50px"