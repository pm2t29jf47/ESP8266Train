const throttleInputButtons = document.getElementById("throttleInput").querySelectorAll("button");
throttleInputButtons.forEach((button) => {
    button.addEventListener("click", (event) => {
        var newNumberValue = getPlusMinusInputValue(event);
        console.log("ThrottleInput value: " + newNumberValue);
    });
});
const volumeInputButtons = document.getElementById("volumeInput").querySelectorAll("button");
volumeInputButtons.forEach((button) => {
    button.addEventListener("click", (event) => {
        var newNumberValue = getPlusMinusInputValue(event);
        console.log("VolumeInput value: " + newNumberValue);
    });
});

function getPlusMinusInputValue(event) {
    const pressedButton = event.currentTarget;
    const minusButton = pressedButton.parentNode.querySelector(".minus");
    const plusButton = pressedButton.parentNode.querySelector(".plus");
    const numberContainer = pressedButton.parentNode.querySelector(".number");
    const numberValueContainer = pressedButton.parentNode.querySelector(".numberValue");

    const numberValue = parseFloat(numberValueContainer.textContent);
    const newNumberValue = pressedButton.classList.contains("plus") ?
        numberValue + 20 :
        numberValue - 20;
    numberValueContainer.textContent = newNumberValue;

    if (newNumberValue === 0) {
        minusButton.disabled = true;
        numberContainer.classList.add("dim");
        pressedButton.blur();
    } else if (newNumberValue > 0 && newNumberValue < 100) {
        minusButton.disabled = false;
        plusButton.disabled = false;
        numberContainer.classList.remove("dim");
    } else if (newNumberValue === 100) {
        plusButton.disabled = true;
        pressedButton.blur();
    }

    return newNumberValue;
}