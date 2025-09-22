module.exports = [
  {
    "type": "heading",
    "defaultValue": "Titanium"
  },
  {
    "type": "text",
    "defaultValue": "<p>by astosia</p>"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Features"
      },
      {
        "type": "toggle",
        "messageKey": "VibeOn",
        "label": "Vibrate on Bluetooth disconnect during Quiet Time",
        "description": "If set to off, will still vibrate on disconnect, but not during quiet time",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Date visible",
        "messageKey": "EnableDate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Seconds hand visible",
        "messageKey": "EnableSecondsHand",
        "defaultValue": true
      },
      {
        "type": "slider",
        "messageKey": "SecondsVisibleTime",
        "defaultValue": 135,
        "label": "Seconds hand visibility",
        "description": "Visibility of seconds hand, in seconds. Timer starts after a tap, shake, or watchface launch. Setting to max value will keep the seconds hand permanently on",
        "min": 15,
        "max": 135,
        "step": 15
      }
    ]
  },
  {
    "type": "section",
    "capabilities": [ "COLOR" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colours"
      },
      {
        "type": "color",
        "label": "Background Colour",
        "messageKey": "BackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Shadow Colour",
        "messageKey": "BackgroundColor2",
        "defaultValue": "AAAAAA"
      },
      {
        "type": "color",
        "label": "Tickmark Border Colour",
        "messageKey": "TextColor2",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "6 & 12 Digits Border Colour",
        "messageKey": "TextColor3",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Hours Hand Fill Colour",
        "messageKey": "HoursHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Hours Hand Border Colour",
        "messageKey": "HoursHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minutes Hand Fill Color",
        "messageKey": "MinutesHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minutes Hand Border Color",
        "messageKey": "MinutesHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Seconds Hand Colour",
        "messageKey": "SecondsHandColor",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Date Colour",
        "messageKey": "DateColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Quiet Time and Bluetooth Icon Colour",
        "messageKey": "BTQTColor",
        "defaultValue": "000000"
      }
    ]
  },
  {
    "type": "section",
    "capabilities": [ "BW" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colours"
      },
      {
        "type": "color",
        "label": "Background Colour",
        "messageKey": "BWBackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Shadow Colour",
        "messageKey": "BWBackgroundColor2",
        "defaultValue": "AAAAAA",
        "allowGray": true
      },
      {
        "type": "color",
        "label": "Tickmark & Digits Colour",
        "messageKey": "BWTextColor2",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Hands Fill Colour",
        "messageKey": "BWTextColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Hands Colour",
        "messageKey": "BWTextColor2",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Date Colour",
        "messageKey": "BWDateColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Quiet Time and Bluetooth Icon Colour",
        "messageKey": "BWBTQTColor",
        "defaultValue": "000000"
      }
      //,
       // {
       //   "type": "radiogroup",
       // "messageKey": "BWTheme",
       //   "label": "White or Black",
       //   "description": "<img src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAS4AAACoCAYAAABe3gMyAAAAxHpUWHRSYXcgcHJvZmlsZSB0eXBlIGV4aWYAAHjabVDRDUMhCPx3io4gHPpwHNtnk27Q8YuiybPtJcIJeAKhvV/PcOtgkiDp0FxyjgYpUrga0eiow1KUYQe4TUZ7PNT1iC0E8/CrZve04vPB8lSNpYuQPmbivieKTH39EmJ36B11fk6hMoXAnqApUH2smIse1xHuLe5QP6Eb0b3tn/th2zuT/QPmBoJtCoB6A+gnBVQjxSwh90IrqJARSVgj2UL+7WkhfAB77VmMECyx4QAAAYVpQ0NQSUNDIHByb2ZpbGUAAHicfZE9SMNAHMVfU6UqFRU7iDhkqA5iQVTEsVSxCBZKW6FVB5NLv6BJQ5Li4ii4Fhz8WKw6uDjr6uAqCIIfIO6Ck6KLlPi/ptAixoPjfry797h7Bwi1ElPNjjCgapaRiEbEdGZV9L2iG/0YwCTGJWbqseRiCq7j6x4evt6FeJb7uT9Hr5I1GeARicNMNyziDeLZTUvnvE8cYAVJIT4nnjDogsSPXJcdfuOcb7DAMwNGKjFPHCAW820stzErGCrxDHFQUTXKF9IOK5y3OKulCmvek7/Qn9VWklynOYIolhBDHCJkVFBECRZCtGqkmEjQfsTFP9zwx8klk6sIRo4FlKFCavjB/+B3t2ZuespJ8keAzhfb/hgFfLtAvWrb38e2XT8BvM/Aldbyl2vA3Cfp1ZYWPAL6toGL65Ym7wGXO8DQky4ZUkPy0hRyOeD9jL4pAwzeAj1rTm/NfZw+ACnqavkGODgExvKUve7y7q723v490+zvBwoScuOH/I4/AAANdmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNC40LjAtRXhpdjIiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iCiAgICB4bWxuczpzdEV2dD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlRXZlbnQjIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6R0lNUD0iaHR0cDovL3d3dy5naW1wLm9yZy94bXAvIgogICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iCiAgICB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iCiAgIHhtcE1NOkRvY3VtZW50SUQ9ImdpbXA6ZG9jaWQ6Z2ltcDo1MDM5YmUzNC03NjU4LTRhMzUtOGM4OS00ZTZiOGYzNTJiMTgiCiAgIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6ZTdmNDg1NjQtMWUwOS00NDFiLThhYzgtMjMyZDAyYmJjNDY1IgogICB4bXBNTTpPcmlnaW5hbERvY3VtZW50SUQ9InhtcC5kaWQ6YjUxYjMxZTItMTc3MS00MjI2LWIxZDgtZjVmMTdhM2VmMjM4IgogICBkYzpGb3JtYXQ9ImltYWdlL3BuZyIKICAgR0lNUDpBUEk9IjIuMCIKICAgR0lNUDpQbGF0Zm9ybT0iV2luZG93cyIKICAgR0lNUDpUaW1lU3RhbXA9IjE3NTgwNjMwMjI4MDAyNDciCiAgIEdJTVA6VmVyc2lvbj0iMi4xMC4zOCIKICAgdGlmZjpPcmllbnRhdGlvbj0iMSIKICAgeG1wOkNyZWF0b3JUb29sPSJHSU1QIDIuMTAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjU6MDk6MTZUMjM6NDk6NTMrMDE6MDAiCiAgIHhtcDpNb2RpZnlEYXRlPSIyMDI1OjA5OjE2VDIzOjQ5OjUzKzAxOjAwIj4KICAgPHhtcE1NOkhpc3Rvcnk+CiAgICA8cmRmOlNlcT4KICAgICA8cmRmOmxpCiAgICAgIHN0RXZ0OmFjdGlvbj0ic2F2ZWQiCiAgICAgIHN0RXZ0OmNoYW5nZWQ9Ii8iCiAgICAgIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6OWE3YjlmMzAtYTI0ZS00OTI5LThmMzItNDljNWY0MWY5ODMyIgogICAgICBzdEV2dDpzb2Z0d2FyZUFnZW50PSJHaW1wIDIuMTAgKFdpbmRvd3MpIgogICAgICBzdEV2dDp3aGVuPSIyMDI1LTA5LTE2VDIzOjUwOjIyIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+CiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAKPD94cGFja2V0IGVuZD0idyI/Pkh7tOwAAAAGYktHRACqAKoAqiE4rggAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfpCRAWMhahmevmAAAQrklEQVR42u1d7Y4sKQjtrtT7v3Ltn52bmppSAUW+DslmkrvTPYpwOCDq97qu6wOBCOX7/X6FH4XdQcRyQAUQCATABYFAIAAuCAQC+S1n5sm1yi/Psp68TNMXlA9rSmvdn3amZR9a9gzgMjKktwW9/9sqQ/r5zu/3C/AqLD928GYD939bBTQ/33ldV3rwQqqoEP2u6+qCJQSiwY6+328XLMG4kkbE1ZEPAqFkAKuZfgUB49qcIkAguwARjKsQXZ8FG6SGEAkzn7WbakESjAsCgYBxRY14EIgFw4cEBC6NVgRPhqRpoF50Vz1F8xg4Ne3Bi+5OL2BhVdDe1RSoCYboF5M7tVVBe1cTtCYYWvaLHdbOd+952s1uNMZBBeIVxVgv0S8q01qhM4nNaIyDCsQrNp88sP3TiyFZsoco28hv46vQJZ1Vj1HaZrz4qTnjajGebNF9Rw0DoAX99RhRVn89rQynp9w3NB8h/M93zip0x4JQxvn8nZExosY1r/c3u6Ta6ixY7ABQyjifv0Ox01KM6y09g/PJ2AL0NpeegbXK2Jal3g6vjsmNfJmZAQDKBtCo7DhTUPQGUK5SxaeiWsbBNRqr3UnuuDjjbI0HYDYfGFrBkBskrXYnuePijLM1Hi9gdngyJorSNBXmAQRaYI3UZl/qPRtAPQTLWbbF8ctyjIsCRhyjuRdKpUpevTj3uXHHh4L8/gAhDZKzQXU1eD0vFeSMz2tB3hVwtYypR+O5oDVS9I6FeI6PwqzAtvYHz17ZovW5FjiMAGDnDnZvnK35ed44O7wYzkydZ7QonpzjbZwjowHbsknRekGEAgae7e4+j2hs6/P5fL4WD8KOolpvR60XQSiMxppNjjYink7A2V20mHOkB2FHLH60o0a1uwjBsqUXqi5WpslhGJck374rlgJWPWC0+EkxnpYRrajZQPj1UgrD4qzr7p9UUL3vsnJZlRVYu+rjGimuB14jR+cyNe2f0rQY4KSXMo52s7npfiS7k/pkuRoXZTexlXdLFiOa8XAK8mBb61hXT+9UsPIMXpy6H2VzwQrQTGpcI0OS1HcyR//e3K1BK1KNa+R0kvpOFuHO3bqed+xwQiqzGNWkKqYvK3YXq+lPopP7Zyilh+y6W7G7qKm/Q1sJHDr9xiCqHcJePfeKaaTkqFgPyCoAf8vWpHPXZmTHDkek1mh6taBKzrdq7pVrX9S5j2pAldjqzNnG3WnkthoXp3/JYx9WdtYx8Xdc17h6ThSliTRa0N1yEsWyOA/QSgGQoYrzUZpHIZtSRe6iA7R8pQVZ0mqAVp712gJc3EK8pIkUEiuN9DQXgFauNPLYPTAwLIDW7jkBtHKB1nLgGg0QoAXQ2j03gFY+0Pp8DHYVAVrpAMNtcR6glVeOTcYN0IKYRHmAFoDrFxABtHKmW6P/f//PAowAWrr6tfBNymHu+3+/UkXu8RLKoWeAVizQkly38/l8plJF7vES6qV2AK05ANmlq9G6NC/NvK7r4tztTjV4gFYsltW7VXUAHF+5zdLuducaPEBrnvVo64sSgHp2+v3ciqSjw6ZU8AJoxWdZFDv4/3e+QsO9euOgsq/Ww8EeQcvrwW0L0OIC1p/Pv1F2quEiPczLwIigNQ1cIzuTXCPsDbSEKXhK0OLoqGeLJ5XKoaZVD7RmUrlVqSv1Ir+IoEWp8SBNbfzOs8ZFMVYOeEFyAFbr92b6uEapE5UNREwPPTAd6zFw1vf5e6ziPIXWg3HVAKzb/1MvzvcMPHJNy7LmZQlaM4D1I2eLjvcu83tLHwFaNoAj0fGqQLU6LWylTq30sRUsITFSwhmm/XrkRxqJnyAG0WVIUlBZXBpYduRnJhJ7BC0Ko7LqnbJgW6tKA03g4hjwTA8YZB6wuDqX9m3tAC6OAXvZDYsGXNagNdMGcZeDO8k3wwZQ7QWtN53PGuBzbS3XlWJjb+PNltJnAa3RmJ7gTRnXwf1isCx70Fqh83ud0gtgUYKkhI1GWeNKr1hJAasLXBRm9fwdFEX3gpZU573t+VGby46OagqzWgXelcSabfWePxvVbl8DFqXGxYl0MCR9wFoRramfJdTE1GpcUl14Bg2LtghPKSJ1rqOaGOkiwZbxam+XA7TsNkQoJyW0gGtkvBEK81xn1QSvSPqinpQ4JRP/+WIAVU7Q8hKEWgeoUZbICVqc8YkvEkSzX17Q8qyniK9BWRXgo4EWRw6p8ewEL8tbN3fM62lsAK22I2YNmqvBLTNokVPFnvFoOs9rq3+S7WOwLIAWxJBxWea6UQ0YoOU3WGZIF7OzrV+Mi6JYK+Pp7bhQd79W1FUAWropDec9zozyrOHN9ullAq2nnZwjA3o7ua8NXj3naC2oxnju3eWz3z3bk2QJWppsodeYiDSxtvSC2klBbWvG1RrfbvZVlWXtGsPotpHo4LWCUVViW73xH1yle6ox9HbgvCwa6llrnL0K45Ky2wp1LRFw7TQe6Vt7HMQGaMVJF5Am+mDEIYHLknFRGJQn9oX+LIDWaka1KsUqBVwWoCW9wsSafXFYFu42i1ee8M62qrDSPw/CejOimVsoZm+w4EbHSqnhqncVR+uUMU0cPf4hteMKoPVvY65lQL0HCawp9goA436OcEtCetB608eK2yF6Nc0q4LXC9rMC15t9DK+18UTXd7Mv7p1VknQ1egqkfa1N1sL8LHBVTRF/5IhkPDMFeE5xfAZMUc9at9bYTeSnmgAuh4xLwmio6Rt38amfwa6hPCCgMP/XXmA7ARnXKvbFTSc5vze6hx+GB8a1kiVVZaMhGZcmgI0+Q2VZAK11jAuCFHGKcUVa1NbizgCP1FBQz5pjXBCA1h/gut8u+uawkSLfiH1ZzCGb4/VsRbJW9/+qpIlaNhb1p8Rehs+TRY18Gifvud9ZgS1otUNUAC+pffSu/In2UyVVjGw8q9MzKWjd7/Ty/NNrmpiZca2owUYGramX2LMyLm0GRgWt7Eakzbiq1a64dlw1rf5+CB3MWVKfmVSP2yPWo8VUI2t9jkq/R3+nB1oMXU2fVfQUdDwAV+/ExuiyxZEeWzZx/3dJn+Qzw5D2WlLN4uQ6fmTw4kQl6pXRrYV7+56ncfRAo6Xnt3E9fz4/31szjdMEluuWCbTebOENIFpBa2YtJQ3ZszufHGw5uTclZAAvavSgOhAX5KSOu0P3K27EWOkYmlcdR8kIeuueJU3kZjcn13kyRbsV86G+UmM5XkkzrJfghF4uml1oBMuVpRbqd1D/xjlj/Fmo+kxjaS/yaTwZz3VkqQF6AQyAFm2dVvvniu+j3poiCdAnepPoTst5Nk3rFZcd9/3vaJWY3U3LzJ44usqyq8gN8qemsUU2mCd4tXb3RpFvFTWWgpf0UVFPTlDhAdheAOzpQwJaVOajkSquYvnfC1yctSi9beOK9RjNiwQrCPq4ZHLAdHgGNAItnK2D7GSTVe0OwMWoJfR+b3URHlLX5rhpX0Wmf46KgdWU0ttpBCitqzf1HBa3Q4BxjeyFVOOq2kuz4/HOBDrCWcXF4M15mKWqkG9AhUAsmEQZBkE8GQFfJAIX6DrEyrFhdxAwLggYl2O2BQHjgoBxAbTAuLC7AwHjAmj5krOlyN79TpC6YNJywlUOPXucBWw0d2r9M++TYowAL0gPYLS+K5ODgm2tDZjo4zJgGMl0pHpWMdsleRTQav3+ysARXY4qxgNBShQ9eEKYwAUjglg5bQame7/fDD60GbhQ44IAvGyZE0oXYFwQpIth5g/5KycH2aHEuqxnhw28FZ9LPnaK7KZpD3+Ai2KcUCgMaDWIUV9JAuNHwLz/+8kxQvRzQVavveRFb0htEPt8BDegArQgSJ3yMN6ockiMCAKxjriYb20RMS4oFQKBhAIupIsQDykTBMBFoq3PU/uQ+KlIr5PbQ5f3dV2lH26hpJHR9PGzpr0LHaZfsm69ISh5Khzi0/BbgPB87Wjn2vZeWnoba4VAmvGwee/l954NNIFLaqyIiPGi9b3Rs2VcuwBsZKytv5+17prJn1rMubXmIwA7ubWEtyfA7/8O8IplSG/r1wOwnUyw9/ef48SmUazA+bZ+PQBjpYoUwMoe9aoB2QjArNMLSpoLiQdkIwB7CunO+RFoVakzZAMoSp3Sal3firTV66qRG1HvAEXBj9GcSMV5DmAh+sWOeB7YlwSwsgbOrIzyjVlx2NfwdoiewQK0chnNqDCvDWBUMML97bmC5Kgw/2avJwALAMbdWVwNYACs2gBG3Vn8Vea4ruui1g6qAVa1A63CWqb4sQxqrbQ6YPXmn+EBDUlp4ARgQVq0nVL/0gJNANanm8Znnh+l/nWsrvZD4oPXjt28KLubkH3gxWH8B/WLemeIYFQ5UsMegO06rzr6uwii+VJfDu78C3if28OcbxX+kXFkprEValzUQnsHqL5C4716NkR5NLX6M/SU/+d9PtzHcf/Nk1Oclxg8gCtPHeXNYWZespY0MmvV2KKxlCwFekoAepv7+fbhyiyrMn0fravGffMcp6t8rCyrz1EeQuk2oHKUAtCqV3vYwWpX/i4kZmZDlQNqg0Ag0UQFuBAZ80Q2MEb/c/Vei9UY06HhHEgl/Rp7VvCqUP+KOj8NTDi8DxACBwdo5QDdldhweB0YBI4O0KKlYFH8biVGfC+gDYmaQ01NHYn7uKC9OcDKlE6apooZ8/MMoAsnh2TT9xbGNUMRrVmPZQo8O/cdY/fMuGbSyAjd6FpjnP3eHem7OnBJnaf6wdpV89cGL6/AJXWe6hcJrJq/Nngd2s634vqTDJelzTCtGR1WLF6vmPPbLQWVQGv2JXPtgHnsUgSHWeCV7Pacqe/OVdadVCd4vYruf1LfDgFcs6hcjWmNmBcY1R5GVn03meJ31u1Ph2dnrcoeJu7GgkDnJXzvsFQSl21VPIsG1mXPtirp9s3XPLKu0xNo7QQozauHdzje2wMDOL0gA62dAKW1PurtB//b1tuDKhaB003n/OhGx5XD1KhhaNVFRhHQevmid86P9OqpP2rXd1IYvzULdVfj2qmclU5/b1vQGDeOH9kETu/s6N62oDFurxtkpzejoShvZoGei6wR+Vam0ZSaDIBMZ82eYDZjK09702D6K9NoSg3aEsgOr4YUrSC6oz4H1gW9UlK6Cn55ejIabbalxZZ+5rBiUd+K7GBdsdmWFltaea3NW5HdM+s6PBqSlL1EYWkzZ+jAuvTZluRcXgawpmYSHvzs9GA01OMDXg1k9bg4rQ2t9gjImvTKc4/c6nFxWhta7RHpgavX1KYBBj3H1try1nC4HkgBvOTro1Ur6jm2VouPhq/2QMoCvE5PhtSbvNYjtavqA5xxrHh89f55dM6vSRElgCDV/ap6KGcc0sdXW/O1DJQu31WMuLOj3ccVVTdRwSzKeLX7uLzqxsWuojZj2JEmWqRuVsctsqSM2uuzI020SN08HC87LA2HS9s9MxFu1Ju9HHCl7pAiygOYddDgsvzZywFX6i4s43pGBivHW9XHVVF3UcHLw/b+qj6uirpzVZyP/Dcs/i7AynewiQqI3lmmaaoIgUAgYRlXlegLgYAxg3G5Mp5drRAQBMvnd1S8HfjzcXSRoIXRaHTnV4um0S8StAiSGt351bKHksClvcCVVArg8mMbldh+2VRR63oc1C0gu8Fl9vHWiFK6OA+QgWRj+2BcEAgEAuCCQCAQABcEAikq/wHM5W6bdyi5ZgAAAABJRU5ErkJggg==' />",
       //   "options": [
       //     {
       //       "label": "White Background",
       //       "value": 1
       //     },
       //     {
       //       "label": "Black Background",
       //       "value": 2
       //     },
       //   ]
       // },
      //  {
      //    "type": "toggle",
      //   "label": "Shadow on",
      //   "messageKey": "BWShadowOn",
      //   "defaultValue": true
      // }
    ]
  },
  // {
  //   "type": "text",
  //   "capabilities": [ "COLOR" ],
  //    "defaultValue": "<img src='data:image/png;base64,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' />"
  //  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
