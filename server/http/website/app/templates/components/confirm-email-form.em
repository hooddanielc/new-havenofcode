.login-form-container
  unless success
    if failed
      p We are sorry, but since you got the password wrong, you need to register again.
      link-to 'index.register'
        button.btn.btn-primary.btn-lg class="incorrect-submit" type="submit" Try Again

    else
      form{action "confirm" on="submit"}
        .user-icon
          span.glyphicon.glyphicon-user
        unless loading
          .form-group
            label for="email" Email Address
            = input value=email type="email" class="form-control input-lg" id="email" placeholder="Email"

          .form-group
            label for="password" Password
            = input value=password type="password" class="form-control input-lg" id="password" placeholder="Password"

          button.btn.btn-primary.btn-lg type="submit" Next
        else
          .form-group.text-center
            .fa.fa-circle-o-notch.fa-spin
  else
    .user-icon
      span.glyphicon.glyphicon-user
    p.text-center.success Congratualations, registration successful.
    link-to 'index'
      button.btn.btn-primary.btn-lg Continue
