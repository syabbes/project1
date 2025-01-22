#include <Adafruit_PCF8574.h>
#include <stdlib.h>
#include <string.h>
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

int buttons[2][5] = {
  {3, 2, 1, 0},
  {4, 5, 6, 7}
};
//int buttons2[] = {26, 27, 13, 14, 12};
int leds[] = {16, 17, 18, 19};

struct speler
{
  bool beurt;
  int reeks[50];
  int reekslengte;
  int reeksherhaling[50];
};

struct dobbelsteen
{
  String som;
  int oplossing;
};

speler spelernummer[2];

bool start = true;

// set up io expander
Adafruit_PCF8574 pcf;

Adafruit_PCF8574 pcf_dobbelsteen;

// Stel het I2C-adres van de LCD in op 0x27 en de afmetingen op 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
  // Start de I2C communicatie
  Wire.begin();

  lcd.begin(20, 4);
  lcd.backlight();
  lcd.init();

  // Toon de initialisatieboodschap
  lcd.setCursor(3, 1);
  lcd.print("MEMORYOVERFLOW");
  delay(5000);
  lcd.clear();

  // start io expanders
  if (!pcf.begin(0x20, &Wire))
  {
    Serial.println("Niet gelukt om io expander te vinden");
  }
  if (!pcf_dobbelsteen.begin(0x21, &Wire))
  {
    Serial.println("Niet gelukt om io expander dobbelsteen te vinden");
  }
  // put your setup code here, to run once:
  Serial.begin(115200);
  // loop over buttons en leds om ze actief te maken
  for (int i = 0; i < 4; i++)
  {
    pcf.pinMode(buttons[0][i], INPUT);
    pcf.pinMode(buttons[1][i], INPUT);
    pinMode(leds[i], OUTPUT);
  }
  // zet alles klaar voor de spelers
  spelernummer[0].beurt = false;
  spelernummer[0].reekslengte = 2;
  spelernummer[1].beurt = false;
  spelernummer[1].reekslengte = 2;

}

void loop()
{
  // put your main code here, to run repeatedly:

  // is het spel net begonnen?
  if (start == true)
  {
    // geef instructies op scherm (speler 1 mag reeks intoetsen voor speler 2)
    Serial.println("Speler 1 mag reeks intoetsen voor speler 2 met de lengte van 2");
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("SPELER 1 MAG REEKS");
    lcd.setCursor(1,1);
    lcd.print("  INTOETSEN VOOR ");
    lcd.setCursor(5,2);
    lcd.print(" SPELER 2");
    lcd.setCursor(0,3);
    lcd.print("MET EEN LENGTE VAN 2");

    // loop over reeks speler 2
    for (int i = 0; i < 2; i++)
    {
      //check welke knoppen er worden ingetoetst
      int ingedrukt = false;
      while (ingedrukt == false)
      {
        for (int j = 0; j < 4; j++)
        {
          //Serial.println(j);
          if (pcf.digitalRead(buttons[0][j]) == LOW)
          {
            Serial.println(buttons[0][j]);
            // verander eerste plek van reeks naar j
            spelernummer[1].reeks[i] = j;
            // laat het huidige ledje branden als indicator
            digitalWrite(leds[j], HIGH);
            delay(500);
            digitalWrite(leds[j], LOW);
            ingedrukt = true;
            delay(200);
            break;
          }
        }
        //while (1);
      }
    }

    // geef instructies op scherm (speler 2 mag reeks intoetsen voor speler 1)
    Serial.println("Speler 2 mag reeks intoetsen voor speler 1 met een lengte van 2");
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("SPELER 2 MAG REEKS");
    lcd.setCursor(1,1);
    lcd.print("  INTOETSEN VOOR ");
    lcd.setCursor(5,2);
    lcd.print(" SPELER 1");
    lcd.setCursor(0,3);
    lcd.print("MET EEN LENGTE VAN 2");

    // loop over reeks speler 1
    for (int i = 0; i < 2; i++)
    {
      //check welke knoppen er worden ingetoetst
      int ingedrukt = false;
      while (ingedrukt == false)
      {
        for (int j = 0; j < 4; j++)
        {
          if (pcf.digitalRead(buttons[1][j]) == LOW)
          {
            Serial.println(buttons[1][j]);
            // verander eerste plek van reeks naar j
            spelernummer[0].reeks[i] = j;
            // laat het huidige ledje branden als indicator
            digitalWrite(leds[j], HIGH);
            delay(500);
            digitalWrite(leds[j], LOW);
            ingedrukt = true;
            delay(200);
            break;
          }
        }
      }
    }
    start = false;
    spelernummer[0].beurt = true;
  }
  // spel kan nu echt beginnen


  // check wiens beurt het is
  if (spelernummer[0].beurt == true) // speler 1
  {
    // geef instructies op scherm (de speler moet proberen zijn reeks in te toetsen)
    Serial.println("Speler 1 mag zijn reeks proberen in te toetsen");
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("SPELER1 TOETS DE");
    lcd.setCursor(6,1);
    lcd.print("REEKS IN");
    // get input van de speler
    reeksherhaling(0);

    // check of reeks correct is
    if (controle(0) == true)
    {
      // feliciteer speler 1 op het scherm
      Serial.println("Reeks is correct");
      lcd.clear();
      lcd.setCursor(1,1);
      lcd.print("REEKS IS CORRECT!");
      delay(3000);
    }
    else
    {
      // speler 1 heeft verloren, speler 2 heeft gewonnen
      Serial.println("Reeks is incorrect! Speler 2 heeft gewonnen");
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("REEKS IS INCORRECT");
      lcd.setCursor(3,1);
      lcd.print("SPELER 2 HEEFT");
      lcd.setCursor(5,2);
      lcd.print("GEWONNEN!");
      delay(5000);
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("GAMEOVER!!");
      gameover();
    }
    // geef instructies op scherm (los som op en leg de blokjes op juiste manier)
    Serial.println("Los de volgende som op en leg de blokjes neer op de goede plek(doppelsteen patroon):");
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("LOS DE SOM OP:");
    
    // som
    dobbelsteen som = som_generator();
    Serial.println(som.som);
    lcd.setCursor(5,2);
    lcd.print(som.som);

    // weergeven op het scherm: "Druk een knop in om uw antwoord te bevestigen"
    Serial.println("Druk een van de knoppen in om uw antwoord te bevestigen");

    // hieronder het 2e gedeelte van het spel: dobbelsteen en blokjes

    // wacht totdat er een knop wordt ingedrukt
    bool ingedrukt = false;
    Serial.println(ingedrukt);
    while (ingedrukt == false)
    {
      for (int i = 0; i < 4; i++)
      {
        if (pcf.digitalRead(buttons[0][i]) == LOW)
        {
          digitalWrite(leds[i], HIGH);
          delay(500);
          digitalWrite(leds[i], LOW);
          ingedrukt = true;
          Serial.println("bevestigd");
        }
      }
    }
    // controleer doppelsteen
    if (dobbelsteen_controle(som.oplossing) == true)
    {
      // geef instructies op het scherm: "goed, u mag de reeks van de ander langer maken"
      Serial.println("goed je mag de reeks van de ander langer maken");
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("GOED, JE MAG DE");
      lcd.setCursor(1, 1);
      lcd.print("REEKS VAN DE ANDER");
      lcd.setCursor(4, 2);
      lcd.print("LANGER MAKEN");
      delay(3000);
      verleng_reeks(0);
    }
    else
    {
      // weergeven op het scherm: "helaas, de ander is weer aan de beurt"
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("HELAAS, DE ANDER");
      lcd.setCursor(0, 2);
      lcd.print("IS WEER AAN DE BEURT");
      Serial.println("fout");
      delay(3000);
    }
    // andere speler aan de beurt
    spelernummer[0].beurt = false;
    spelernummer[1].beurt = true;
  }

  // als speler 2 aan de beurt is
  else // speler 2
  {
    Serial.println("Speler 2 mag zijn reeks proberen in te toetsen");
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("SPELER2 TOETS DE");
    lcd.setCursor(6,1);
    lcd.print("REEKS IN");
    // get input van de speler
    reeksherhaling(1);

    // check of de reeks correct is
    if (controle(1) == true)
    {
      // Feliciteer speler 2 op het scherm
      Serial.println("Reeks is correct");
      lcd.clear();
      lcd.setCursor(1,1);
      lcd.print("REEKS IS CORRECT!");
      delay(3000);
    }
    else
    {
      // speler 2 heeft verloren, speler 1 heeft gewonnen
      Serial.println("Reeks is incorrect! Speler 1 heeft gewonnen");
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("REEKS IS INCORRECT");
      lcd.setCursor(3,1);
      lcd.print("SPELER 1 HEEFT");
      lcd.setCursor(5,2);
      lcd.print("GEWONNEN!");
      delay(5000);
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("GAMEOVER!!");
      gameover();
    }
    // geef instructies op scherm (los som op en leg de blokjes op de juiste manier)
    Serial.println("Los de volgende som op en leg de blokjes neer op de goede plek(doppelsteen patroon):");
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("LOS DE SOM OP:");
    
    // hieronder het 2e gedeelte van het spel: dobbelsteen en blokjes
    // som
    dobbelsteen som = som_generator();
    Serial.println(som.som);
    lcd.setCursor(5,2);
    lcd.print(som.som);

    // weergeven op het scherm: "Druk een knop in om uw antwoord te bevestigen"
    Serial.println("Druk een van de knoppen in om uw antwoord te bevestigen");

    // wacht totdat er een knop wordt ingedrukt
    bool bevestiging = false;
    while (bevestiging == false)
    {
      for (int i = 0; i < 4; i++)
      {
        if (pcf.digitalRead(buttons[1][i]) == LOW)
        {
          digitalWrite(leds[i], HIGH);
          delay(500);
          digitalWrite(leds[i], LOW);
          bevestiging = true;
          Serial.println("Bevestigd");
        }
      }
    }
    // controleer doppelsteen
     if (dobbelsteen_controle(som.oplossing) == true)
     {
      // geef instructies op het scherm: "goed, je mag de reeks van de ander langer maken"
      Serial.println("goed, je mag de reeks van de ander langer maken");
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("GOED, JE MAG DE");
      lcd.setCursor(1, 1);
      lcd.print("REEKS VAN DE ANDER");
      lcd.setCursor(4, 2);
      lcd.print("LANGER MAKEN");
      delay(3000);

      verleng_reeks(1);
     }
     else
     {
      // weergeven op scherm: "helaas, de ander is weer aan de beurt"
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("HELAAS, DE ANDER");
      lcd.setCursor(0, 2);
      lcd.print("IS WEER AAN DE BEURT");
      Serial.println("fout");
      delay(3000);
     }

    // andere speler aan de beurt
    spelernummer[1].beurt = false;
    spelernummer[0].beurt = true;
  }
}

// hieronder staan alle functies

void reeksherhaling(int a)
{
  // loop over array reeksherhaling heen
  for (int i = 0; i < spelernummer[a].reekslengte; i++)
  {
    // zolang er geen knop wordt ingedrukt, loop over de knoppen heen
    bool ingedrukt = false;
    while (ingedrukt == false)
    {
      for (int j = 0; j < 4; j++)
      {
        // als een knop is ingedrukt, zet de huidige knop in de array reeksherhaling (j)
        if (pcf.digitalRead(buttons[a][j]) == LOW)
        {
          spelernummer[a].reeksherhaling[i] = j;
          // laat bijbehorende ledje branden ter verduidelijking
          digitalWrite(leds[j], HIGH);
          delay(500);
          digitalWrite(leds[j], LOW);
          // zeg dat er een knop is ingedrukt zodat de computer doorgaat naar de volgende plek in de array reeksherhaling
          ingedrukt = true;
          delay(200);
          break;
        }
      }
    }
  }
}



bool controle(int a)
{
  // loop over array reeks en reeksherhaling
  for (int i = 0; i < spelernummer[a].reekslengte; i++)
  {
    // if reeks niet hetzelfde als reeksherhaling, return false
    if (spelernummer[a].reeks[i] != spelernummer[a].reeksherhaling[i])
    {
      return false;
    }
  }
  // otherwise, return true
  return true;
}

void gameover()
{
  Serial.println("reset spel aub");
  while (true)
  {
    // loop forever
  }
}

dobbelsteen som_generator()
{
  // x + y - z = s
  // s = random(1 - 6)
  int x = random(1, 99);
  int y = random(1, 99);
  int s = random(1, 6);
  int z = x + y - s;

  dobbelsteen q;
  String som(String(x) + "+" + String(y) + "-" + String(z));
  q.som = som;
  q.oplossing = s;
  return q;
}

bool dobbelsteen_controle(int oplossing)
{
  // alle mogelijke oplossingen dobbelsteen (1 - 6)
  // oplossing 1 => poort 5
  int oplossing1[7] = {LOW, LOW, LOW, LOW, HIGH, LOW, LOW};
  // oplossing 2 => poort 1, 4 of 3, 7
  int oplossing2[2][7] = {
    {HIGH, LOW, LOW, HIGH, LOW, LOW, LOW}, 
    {LOW, LOW, HIGH, LOW, LOW, LOW, HIGH}
  };
  // oplossing 3 => poort 1, 4, 5 of 3, 5, 7
  int oplossing3[2][7] = {
    {HIGH, LOW, LOW, HIGH, HIGH, LOW, LOW},
    {LOW, LOW, HIGH, LOW, HIGH, LOW, HIGH}
  };
  // oplossing 4 => poort 1, 3, 4, 7
  int oplossing4[7] = {HIGH, LOW, HIGH, HIGH, LOW, LOW, HIGH};
  // oplossing 5 => poort 1, 3, 4, 5, 7
  int oplossing5[7] = {HIGH, LOW, HIGH, HIGH, HIGH, LOW, HIGH};
  // oplossing 6 => poort 1, 2, 3, 4, 6, 7
  int oplossing6[7] = {HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH};

  // sla antwoord op in array
  int antwoord[7];
  for (int i = 1; i <= 7; i++)
  {
    antwoord[i - 1] = pcf_dobbelsteen.digitalRead(i);
    Serial.println(pcf_dobbelsteen.digitalRead(i));
  }
  // vergelijk antwoord met juiste oplossing
  switch (oplossing)
  {
    case 1:
      for (int j = 0; j < 7; j++)
      {
        if (oplossing1[j] == antwoord[j])
        {
          return false;
        }
      }
      break;
    case 2:
      for (int j = 0; j < 7; j++)
      {
        if (antwoord[j] == oplossing2[0][j] && antwoord[j] == oplossing2[1][j])
        {
          return false;
        }
      }
      break;
    case 3:
      for (int j = 0; j < 7; j++)
      {
        if (antwoord[j] == oplossing3[0][j] && antwoord[j] == oplossing3[1][j])
        {
          return false;
        }
      }
      break;
    case 4:
      for (int j = 0; j < 7; j++)
      {
        if (antwoord[j] == oplossing4[j])
        {
          return false;
        }
      }
      break;
    case 5:
      for (int j = 0; j < 7; j++)
      {
        if (antwoord[j] == oplossing5[j])
        {
          return false;
        }
      }
      break;
    case 6:
      for (int j = 0; j < 7; j++)
      {
        if (antwoord[j] == oplossing6[j])
        {
          return false;
        }
      }
      break;
  }
  return true;
}

void verleng_reeks(int a)
{
  // geef instructies op het scherm: "selecteer het lampje die u aan de reeks van de ander wilt toevoegen"
  Serial.println("selecteer lampje die u aan reeks wil toevoegen");
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("VOEG LAMPJE TOE");
  lcd.setCursor(5, 2);
  lcd.print("AAN REEKS");
  
  

  // get id van andere speler
  int andere_speler = a ^ 1;
  // check of reeks langer kan worden gemaakt
  if (spelernummer[andere_speler].reekslengte == 50)
  {
    Serial.println("Je hebt een beter geheugen dan deze computer");
    gameover();
  }
  // wacht totdat er een knop is ingedrukt
  bool ingedrukt = false;
  while (ingedrukt == false)
  {
    for (int i = 0; i < 4; i++)
    {
      if (pcf.digitalRead(buttons[a][i]) == LOW)
      {
        // laat led branden als indicator
        digitalWrite(leds[i], HIGH);
        delay(500);
        digitalWrite(leds[i], LOW);
        spelernummer[andere_speler].reeks[spelernummer[andere_speler].reekslengte] = i;
        spelernummer[andere_speler].reekslengte++;
        ingedrukt = true;
      }
    }
  }
}