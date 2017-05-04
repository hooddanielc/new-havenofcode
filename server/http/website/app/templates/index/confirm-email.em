= consumer-header
h1.text-center Confirm Account

if model.error
  .login-form-container
    .user-icon
      span.glyphicon.glyphicon-user
    .form-group
      p.error.text-center error: {{model.error}}
      link-to "index"
        button.btn.btn-primary.btn-lg type="submit" Home

else
  = confirm-email-form model=model.params