 const buttons = document.querySelectorAll("button");
        buttons.forEach((button) => {
            button.addEventListener("click", (event) => {
                const element = event.currentTarget;
                const parent = element.parentNode;
                const numberContainer = parent.querySelector(".number");
                const numberValueContainer = parent.querySelector(".numberValue");
                const number = parseFloat(numberValueContainer.textContent);
                const increment = parent.querySelector(".plus");
                const decrement = parent.querySelector(".minus");
                const newNumber = element.classList.contains("plus") ?
                    number + 20 :
                    number - 20;
                numberValueContainer.textContent = newNumber;
                console.log(newNumber);
                if (newNumber === 0) {
                    decrement.disabled = true;
                    numberContainer.classList.add("dim");
                    element.blur();
                } else if (newNumber > 0 && newNumber < 100) {
                    decrement.disabled = false;
                    increment.disabled = false;
                    numberContainer.classList.remove("dim");
                } else if (newNumber === 100) {
                    increment.disabled = true;
                    element.blur();
                }
            });
        });