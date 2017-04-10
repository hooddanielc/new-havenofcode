.upload-file-container
  form{action "upload" on="submit"}
    .form-group
      label for="file" File
      = input value=file type="file" class="form-control input-lg" id="file" placeholder="Password"

    button.btn.btn-primary.btn-lg type="submit" Next
