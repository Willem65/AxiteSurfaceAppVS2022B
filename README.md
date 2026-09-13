# AxiteSurfaceApp

**AxiteSurfaceApp** is een C# Windows Forms-toepassing ontworpen voor het aansturen van en communiceren met D&R Axite (en verwante) digitale broadcast-mengtafels via het MBN-protocol (`mbn.dll`) en Windows Audio (`NAudio`).

De applicatie verzorgt de netwerkverbinding met de mengtafel-engine, verwerkt live audio-peak/VU-metering van Windows-geluidskaarten en synchroniseert status-, EQ-, routing- en monitorgegevens in real-time.

---

## 📸 Functionaliteiten

### 🔌 MBN Protocol Integratie (`mbn.dll`)
* **Directe TCP/IP-verbinding:** Verbinding met het D&R Axite systeem via `connectToObjects`.
* **Actuator- & statusuitlezing:** Real-time uitlezen van statusveranderingen via de `retString` callback.
* **Fader- & sensorbesturing:** Aansturing van fysieke/virtuele faders en sensoren via P/Invoke functies.

### 🎙️ Audio Metering & Peak Values (`NAudio`)
* **Endpoint-selectie:** Kiezen van actieve Windows Audio Endpoints via `MMDeviceEnumerator`.
* **Real-time Peak Metering:** Directe uitlezing van piekwaarden (`PeakValues`) voor links/rechts audiokanalen.
* **Logaritmische omrekening:** Geavanceerde berekening van audioniveaus voor een nauwkeurige VU-meterweergave op het fysieke/virtuele oppervlak.

### 🎛️ Real-time Besturing & Status-synchronisatie
* **Kanaalsynchronisatie:** Dynamische verwerking van kanaalstatus, kleurindicaties (`ActColorState`) en motorfaders.
* **CRM (Control Room Monitor):** Ondersteuning voor routing, EQ-processing en monitor control.

### 🔍 Diagnose & Debugging
* **Aanpasbare intervallen:** Instelbare timer-intervallen voor polling en status-updates.
* **Live Debug Log:** Ingebouwde debug-lijst voor het realtime monitoren van object-ID's en waarden (VU- en besturingsdata).
* **Flexibele UI-modi:** Snel schakelen tussen een compacte oppervlakte-weergave en een uitgebreide debug-modus.

---

## 🛠️ Vereisten & Afhankelijkheden

### Ontwikkelomgeving
* **Framework:** .NET Framework 4.7.2+ of .NET Windows Desktop Runtime
* **IDE:** Visual Studio 2019 / Visual Studio 2022

### NuGet Packages
* **[NAudio](https://www.nuget.org/packages/NAudio)** (`NAudio.CoreAudioApi`): Voor de verwerking van Windows Audio Endpoints en VU peak meters.

### Externe Bibliotheken (Native)
* **`mbn.dll`**: Vereiste native C/C++ DLL voor de communicatie met het D&R MBN-protocol.
  > ⚠️ **Belangrijk:** Zorg ervoor dat `mbn.dll` aanwezig is in de uitvoeringsmap (`bin/Debug` of `bin/Release`).

---

## 🚀 Gebruik

1. **Verbinding maken:**
   * Voer het IP-adres van de D&R Axite engine in.
   * Klik op **Connect** om de netwerkverbinding via `mbn.dll` te starten.

2. **Audio-apparaat selecteren:**
   * Kies het gewenste audio-apparaat uit de dropdown-lijst om de VU-meters te koppelen.

3. **Debug tools in-/uitschakelen:**
   * Klik op het schroef-icoon (`pictureBox3`) om te schakelen tussen de compacte bedieningsweergave en het uitgebreide debug/status-scherm.

---

## 📄 Licentie

Dit project is gelicenseerd onder de **MIT-licentie**.
