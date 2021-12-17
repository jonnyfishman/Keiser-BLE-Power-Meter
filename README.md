
<h3 align="center">BLE Power Meter</h3>

  <p align="center">
    Got an old exercise bike
    <br />

  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#license">License</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

I regularly use Entry Central to sign up for sporting events. I tend to leave signing up till the last minute in case other factors mean I can't attend. This php script checks the number of spaces remaining for an event and pushes the result as a message to the Telegram app.

The script is currently designed just to get information from one website [Entry Central](https://www.entrycentral.com/) but can be easily adapted to scrape information from other websites. It uses a very simple implementation of the Telegram API for simplicity.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started



### Prerequisites

* PHP >= 5.3.0
* Telegram app installed
* A Telegram bot ([Creating a new bot](https://core.telegram.org/bots#6-botfather))
* Cron or similar scheduler

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/jonnyfishman/Entry-Central-Telegram-Bot.git
   ```
2. Create or use an existing channel within the Telegram app and send yourself a message.

3. Forward the message to @getidsbot and follow the instructions to get your chat id details ([info here](https://github.com/wjclub/telegram-bot-getids))

![Start @getidsbot](/assets/images/chat_id.jpg)

4. Rename config file
   ```sh
   mv config.template.php config.php
   ```
3. Edit `config.php` to include the information from step 2 and 3.
   ```php
   $Telegram_token = "ENTER LONG TOKEN";
   $Telegram_chat_id = "ENTER CHAT ID, MAKE SURE TO INCLUDE THE MINUS SIGN";
   ```
4. Edit `index.php` with the event details. It is possible to include multiple events. Each event needs a name (as a string) followed by an id (as a number). Place a comma after each id.
   ```php
   $events = array(
        'Event A' => 123456,               
        'Event B' => 654321,
   );
   ```
5. Test your information using the command line. Each event will return 'Successfully sent' or a relevant error message.
    ```sh
    /usr/bin/php index.php
    ```

6. Create a cron job to regular check the event. The one below will check at 10am and 4pm. <i>Be mindful about how often you check for updates. Too often could see your IP being flagged as a potential DDoS attacker.</i>
   ```sh
   0 10,16 * * *    /usr/bin/php /LOCATION OF index.php/index.php  > dev/null
   ```


<p align="right">(<a href="#top">back to top</a>)</p>


## Roadmap

<ul>
  <li>Add CCCD callback for custom settings to detect if sending is required</li>
  <li>Update app and publish</li>
</ul>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>
