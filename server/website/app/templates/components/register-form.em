h1.text-center Create Account

.login-form-container
  form{action "create" on="submit"}
    .user-icon
      span.glyphicon.glyphicon-user
    unless showConfirmation
      unless loading
        .form-group
          label for="email" Email Address
          .input-group
            = input value=email type="email" class="form-control input-lg" id="email" placeholder="Email"
            if emailChanged
              if emailValid
                .input-group-addon
                  span.glyphicon.glyphicon-ok
              else
                .input-group-addon
                  span.glyphicon.glyphicon-remove
            else
              .input-group-addon
                span.glyphicon.glyphicon-asterisk

        .form-group
          label for="password" Password
          .input-group
            = input value=password type="password" class="form-control input-lg" id="password" placeholder="Password"
            if passwordChanged
              if passwordValid
                .input-group-addon
                  span.glyphicon.glyphicon-ok
              else
                .input-group-addon
                  span.glyphicon.glyphicon-remove
            else
              .input-group-addon
                span.glyphicon.glyphicon-asterisk

        .form-group
          label for="password-confirm" Confirm Password
          .input-group
            = input value=passwordConfirm type="password" class="form-control input-lg" id="password-confirm" placeholder="Password"
            if passwordConfirmChanged
              if passwordConfirmValid
                .input-group-addon
                  span.glyphicon.glyphicon-ok
              else
                .input-group-addon
                  span.glyphicon.glyphicon-remove
            else
              .input-group-addon
                span.glyphicon.glyphicon-asterisk

        button.btn.btn-primary.btn-lg class="{{if shakeAnimation 'incorrect-submit' '' }}" type="submit" Next
      else
        .form-group.text-center
          .fa.fa-circle-o-notch.fa-spin
    else
      .confirmation-message
        h3.text-center Almost There!
        p Thank you for your interest, {{username}}. Please confirm your email by clicking on the link in your inbox.
        button.btn.btn-primary.btn-lg{ action 'gotoEmail' } Goto {{emailDomain}}
