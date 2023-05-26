document.addEventListener('DOMContentLoaded', () => {
    const fetchData = async () => {
      try {
        const response = await fetch('/data');
        const data = await response.json();
        handleData(data);
      } catch (error) {
        console.log('Error fetching data:', error);
      }
    };
  
    const handleData = (data) => {
      // Process the received data here
      console.log('Received data from ESP32:', data);
    };
  
    const fetchButton = document.getElementById('fetch-button');
    fetchButton.addEventListener('click', fetchData);
  });
  